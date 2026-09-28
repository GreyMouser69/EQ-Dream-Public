# 7. Backup and recovery

Back up the MariaDB database before every update and at least daily. A logical backup example:

```bash
mysqldump --single-transaction --routines --events -u eqdream -p eqdream | gzip > /secure-backups/eqdream-$(date +%F).sql.gz
```

Also preserve the repository commit hash, the real configuration outside Git, and any user-uploaded or generated server data. Test restoration on an isolated host:

```bash
gunzip -c /secure-backups/eqdream-YYYY-MM-DD.sql.gz | mysql -u eqdream -p eqdream
```

A backup is not complete until its restore has been tested. Never overwrite a live database while diagnosing an issue; restore into a new schema first.