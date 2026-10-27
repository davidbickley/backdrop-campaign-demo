# Backdrop campaign demo

A small Backdrop CMS 1.35 site for a fictional union local, Riverbend Lodge 412 of the Allied Metal Trades Union, organizing at a fictional employer, Harbor Fabrication. It has one custom module, `demo_campaign`, a custom theme, `riverbend`, and a full-width layout, built to show two workflows a union communications team runs every week:

1. **Organizing campaign page with an authorization card.** The card is saved locally, the organizer gets an email, and the card is pushed to the CRM (Salesforce Lead fields and REST path, answered here by a stub). If the CRM is down, the card waits in a queue and cron retries it.
2. **School of Workers sign-up with business manager approval.** The applicant applies, the lodge's business manager gets an expiring one-time link, reviews the details and presses Approve or Decline, and the applicant is notified.

All email is captured in an outbox (Admin → Demo campaign → Outbox) instead of being sent.

## Run it locally

Needs Docker. Create `.env` (gitignored) with `DB_PASS`, `HASH_SALT`, `ADMIN_PASS`, then:

```
docker compose up -d --build
```

The first start installs Backdrop and enables the module. The site is at http://localhost:8088 and the admin user is `admin`. One-time login link: `docker compose exec web bee --allow-root uli --base-url=http://localhost:8088`. To start fresh: `docker compose down -v`.

## Walkthrough (about 10 minutes)

1. **/demo** → open the campaign page link. It carries `utm_source=facebook&utm_medium=paid&utm_campaign=fall-drive`, as a paid post would.
2. Sign a card. Point out: the phone is normalized to 10 digits; a hidden honeypot field and a flood limit (5 cards an hour per IP) stop bots; the authorization checkbox is required; the thank-you page fires a Pirsch custom event when Pirsch is on the page.
3. **Admin → Demo campaign → Cards**: the card shows `synced` with a Lead ID, and its source tags. **CRM log** shows the exact JSON the CRM received. **Outbox** shows the organizer alert.
4. **Settings → Simulate a CRM outage**, sign another card. It still saves and the organizer is still alerted; the card shows `retrying` with `HTTP 503`. Turn the outage off, press **Retry now** (or wait for cron): it syncs on attempt 2. No signature is lost.
5. **/school/apply**: apply for a course. In the Outbox, open the review link. Opening it changes nothing; the manager must press Approve. Press it, then open the link again: it's dead. The applicant's confirmation is in the Outbox.

## Design decisions (and why)

- **Save first, then sync.** The local row is the record of truth; the CRM push is a side effect that can fail and retry (Backdrop Queue API + `hook_cron_queue_info()`), and a card is marked `failed` after 5 attempts so it shows up for a person to look at.
- **CRM-shaped, not CRM-specific.** The payload uses Salesforce Lead field names and the request goes to `/services/data/v62.0/sobjects/Lead` with a bearer token. Going live means pointing `crm_endpoint` at a real org and supplying an OAuth access token. Fields with `__c` would be custom fields created in that org.
- **The approver comes from the lodge record, never the form**, so an applicant can't send their own request to an address they control.
- **Review tokens:** 32 random bytes, only the SHA-256 hash is stored, compared with `hash_equals()`, expire after 7 days, and are cleared once used. The update also checks `status = 'pending'`, so a double-click or a second tab can't decide twice.
- **GET never changes state.** Mail security scanners open links automatically; if the link itself approved the request, a scanner could approve it.
- **Config in CMI** (`config/demo_campaign.settings.json`): campaigns, lodges, courses and the endpoint are configuration, not code, so a new campaign is a config change.
- **Core is patched.** The official Docker image ships 1.34.2; the Dockerfile lays the 1.35.2 security release over it.

## Design

- **Theme `themes/riverbend`:** dark steel and safety amber, Barlow Condensed headings over Barlow body text, all colors and spacing as CSS custom properties in `css/riverbend.css`. Styles Backdrop's own form markup (radios become pill buttons, the authorization checkbox gets its own panel) instead of replacing it.
- **Layout `layouts/riverbend`:** a Backdrop layout template that owns the lodge header and footer, with one full-width content region so each page section can run edge to edge. The module's install hook makes it the default layout.
- **Campaign content is configuration:** headline, issues, quote, unit size and the count of paper cards live in `demo_campaign.settings`. The tally adds paper cards to cards signed online, and pages that show it opt out of the page cache so the count is always live.
- **Photos** from Unsplash under the Unsplash License, credited in the footer: Heber Davis (hero), Cemrecan Yurtman (shop floor, sparks), Spencer Davis (measuring), Rafael Juárez (lathe) and sol (crew).

## Trade-offs and production notes

This is a prototype. These are the choices made for speed, and what a production build would do instead.

- **Markup:** campaign page sections are built as escaped HTML strings in `demo_campaign.pages.inc`. Production would move each section into `hook_theme()` templates so a themer can change markup without touching module logic.
- **Storage:** cards and sign-ups are custom tables, because each card carries sync state (status, attempts, last error). Webform with a custom CRM handler is a reasonable alternative where a team already maintains forms in Webform; making cards entities would add Views-based staff reports.
- **Campaign content:** lives in configuration, so it deploys with the site but needs an admin to change. Production would make Campaign a content type that editors manage.
- **CRM:** the endpoint is a mock in the same site (a loopback HTTP request). A real Salesforce connection needs OAuth, a field mapping agreed with the org's owner, and duplicate checks so a worker isn't created twice.
- **Retries:** failed pushes retry on each cron run with no backoff, and a card marked `failed` alerts no one. Production would add backoff and an alert.
- **Flood limit:** 5 cards an hour per IP. At a sign-up event where many workers share one Wi-Fi or carrier IP, that would block real signers; production would raise or re-key it.
- **Review links:** a token in a URL can appear in server logs or a Referer header. One-time use, expiry and hashed storage limit that; production would also send `Referrer-Policy: no-referrer` on review pages.
- **Live tally:** pages showing the count skip the page cache. At real traffic, a counter updated on save would keep those pages cacheable.
- **Mail:** the outbox replaces site-wide mail for the demo. Production would use SMTP or a mail service.
- **Header and footer:** hard-coded in the layout template. Production would use blocks and menus that editors control.
- **Authorization cards:** real card wording and electronic-signature requirements come from the union's legal counsel and NLRB rules. The checkbox here demonstrates the workflow only.
- **Accessibility:** native inputs, visible focus states and alt text throughout, but no formal audit yet.
- **Hosting:** Docker is only how this copy runs. The module, theme and layout install into any Backdrop 1.x site.

## Files

- `modules/demo_campaign/demo_campaign.module`: routes, permissions, the queue worker, `hook_mail()`
- `demo_campaign.pages.inc`: demo index, campaign page, card form (build, validate, submit), thank-you page
- `demo_campaign.school.inc`: sign-up form, token check, review page and Approve/Decline form
- `demo_campaign.crm.inc`: Lead mapping, push with error capture, the stub endpoint
- `demo_campaign.admin.inc`: Cards, School sign-ups, Outbox, CRM log, Settings
- `demo_campaign.mail.inc`: the outbox mail system class
- `demo_campaign.install`: schema for four tables; routes site mail to the outbox

## Deploy on Coolify

New resource → Docker Compose → this repo. Set `DB_PASS`, `HASH_SALT`, `ADMIN_PASS` and `SITE_HOST` (the demo's domain) as environment variables, assign the domain to the `web` service on port 80, and deploy. `robots.txt` disallows all crawlers; add basic auth in Coolify if you want it private.
