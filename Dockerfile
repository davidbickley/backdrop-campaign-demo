FROM backdrop:1.34.2

# The official image lags core releases; lay the current security release over it.
ARG BACKDROP_CORE=1.35.2
ADD https://github.com/backdrop/backdrop/archive/refs/tags/${BACKDROP_CORE}.tar.gz /tmp/core.tar.gz
RUN mkdir /tmp/core && tar -xzf /tmp/core.tar.gz -C /tmp/core --strip-components=1 \
 && rm -rf /var/www/html/core && cp -a /tmp/core/core /var/www/html/core \
 && cp /tmp/core/index.php /tmp/core/.htaccess /tmp/core/robots.txt /var/www/html/ \
 && rm -rf /tmp/core /tmp/core.tar.gz

# MySQL client for the startup check, plus bee (Backdrop's command-line tool).
RUN apt-get update \
 && apt-get upgrade -y \
 && apt-get install -y --no-install-recommends default-mysql-client \
 && rm -rf /var/lib/apt/lists/*
ADD https://github.com/backdrop-contrib/bee/archive/refs/heads/1.x-1.x.tar.gz /tmp/bee.tar.gz
RUN mkdir -p /opt/bee \
 && tar -xzf /tmp/bee.tar.gz -C /opt/bee --strip-components=1 \
 && ln -s /opt/bee/bee.php /usr/local/bin/bee \
 && chmod +x /opt/bee/bee.php \
 && rm /tmp/bee.tar.gz

# Demo only: keep search engines out.
RUN printf 'User-agent: *\nDisallow: /\n' > /var/www/html/robots.txt

COPY docker/settings.php /var/www/html/settings.php
COPY docker/start.sh /usr/local/bin/start.sh
COPY modules/demo_campaign /var/www/html/modules/demo_campaign
COPY themes/riverbend /var/www/html/themes/riverbend
COPY layouts/riverbend /var/www/html/layouts/riverbend
RUN chmod +x /usr/local/bin/start.sh

ENTRYPOINT ["/usr/local/bin/start.sh"]
CMD ["apache2-foreground"]
