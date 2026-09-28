# 1. Prerequisites

Use a fresh Linux host (Ubuntu/Debian is the documented path) with a non-root service account, MariaDB 10.6+ or MySQL 8.0, Git, Perl, and a C++17 toolchain. The build was documented against GCC 12/clang 14 and MSVC 2022.

Install the Linux build dependencies:

```bash
sudo apt update
sudo apt install build-essential cmake ninja-build git \
  libmysqlclient-dev libperl-dev libboost-dev liblua5.1-0-dev \
  zlib1g-dev uuid-dev libssl-dev mariadb-server unzip
```

Create a dedicated service account and clone the repository:

```bash
sudo useradd --system --create-home --shell /usr/sbin/nologin eqdream
sudo -u eqdream git clone https://github.com/tunaria/NMS-Release.git /home/eqdream/NMS-Release
cd /home/eqdream/NMS-Release
```

Do not run the game services as root. Do not expose MariaDB, telnet, management HTTP, or login APIs to the public internet unless they are intentionally secured.