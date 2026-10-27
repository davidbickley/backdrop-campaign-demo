#!/bin/bash
# Installs Backdrop and the demo module on first start, then runs Apache.
set -e
cd /var/www/html

: "${DB_HOST:?DB_HOST is required}"
: "${DB_PASS:?DB_PASS is required}"
: "${ADMIN_PASS:?ADMIN_PASS is required}"
: "${HASH_SALT:?HASH_SALT is required}"

echo "Waiting for the database..."
for i in $(seq 1 60); do
  if mysql -h"$DB_HOST" -u"$DB_USER" -p"$DB_PASS" -e "SELECT 1" "$DB_NAME" >/dev/null 2>&1; then
    break
  fi
  sleep 2
done

installed=$(mysql -h"$DB_HOST" -u"$DB_USER" -p"$DB_PASS" -N -e "SHOW TABLES LIKE 'system'" "$DB_NAME" 2>/dev/null || true)
if [ -z "$installed" ]; then
  echo "Installing Backdrop..."
  bee site-install --auto \
    --db-name="$DB_NAME" --db-user="$DB_USER" --db-pass="$DB_PASS" --db-host="$DB_HOST" \
    --username=admin --password="$ADMIN_PASS" --email="${ADMIN_EMAIL:-admin@example.org}" \
    --site-name="${SITE_NAME:-Riverbend Lodge 412}" --allow-root
  bee enable demo_campaign --allow-root
fi

# Apply any schema changes from a newer module version.
bee updb --allow-root -y >/dev/null 2>&1 || true
# Aggregated CSS/JS lives on the files volume and is keyed by file list, not
# content, so clear caches on every start or a redeploy serves stale styles.
bee cc all --allow-root >/dev/null 2>&1 || true
chown -R www-data:www-data files

exec "$@"
