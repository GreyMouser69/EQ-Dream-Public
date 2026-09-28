# 3. Database setup

Create a new database and a least-privilege local database user. Substitute strong values; do not reuse these examples.

```sql
CREATE DATABASE eqdream CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
CREATE USER 'eqdream'@'127.0.0.1' IDENTIFIED BY 'REPLACE_WITH_A_LONG_RANDOM_PASSWORD';
GRANT ALL PRIVILEGES ON eqdream.* TO 'eqdream'@'127.0.0.1';
FLUSH PRIVILEGES;
```

Load the clean base database shipped with the server source:

```bash
cd /home/eqdream/NMS-Release/Release-NMS-Server/database
unzip release-peq.zip
mysql -u eqdream -p eqdream < release-peq.sql
```

Apply every public EQ Dream migration in filename order:

```bash
cd /home/eqdream/NMS-Release
for migration in database/migrations/*.sql; do
  mysql -u eqdream -p eqdream < "$migration"
done
```

The base dump is intended to be a fresh world, not a replacement for an existing production database. Back up an existing server before applying any migration. The first account is created through your configured login service; grant administration deliberately:

```sql
UPDATE account SET status = 250 WHERE name = 'YOUR_ADMIN_ACCOUNT';
```

Run `Release-NMS-Server/utils/sql/nms_content_health_check.sql` after import to check the supplied content payload.