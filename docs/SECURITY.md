# Security policy for operators

- Use unique random passwords for MariaDB, login services, world keys, web APIs, and operating-system accounts.
- Bind MariaDB and administrative services to loopback/private networks. Allow only required public game ports through the firewall.
- Run each process as an unprivileged service account.
- Keep secrets in files outside the repository with restrictive permissions.
- Keep operating system, MariaDB, and server dependencies patched.
- Back up before updates and test restores.
- If a secret is ever committed, revoke/rotate it; deleting the file is not enough.

For public issue reports, remove player names, IP addresses, hostnames, credentials, database rows, and full logs unless they are necessary and consented to.