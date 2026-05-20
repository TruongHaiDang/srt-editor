# SubtitleEditFree

SubtitleEditFree is a desktop subtitle editor built with Qt6 and C++.

## Features

- Open and edit `.srt` and `.vtt` subtitle files
- Modern Qt6 user interface
- Linux desktop integration
- GNOME application launcher support
- Custom application icon support

---

# Project Structure

```text
project/
├── assets/
│   └── icon.png
├── build/
│   └── SubtitleEditFree
├── scripts/
│   └── install.sh
├── src/
├── inc/
├── CMakeLists.txt
└── README.md
```

---

# Requirements

Ubuntu packages:

```bash
sudo apt update && sudo apt install -y build-essential cmake qt6-base-dev qt6-tools-dev
```

---

# Build

Create build directory:

```bash
mkdir -p build && cd build
```

Configure project:

```bash
cmake -S .. -B . -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
```

Build executable:

```bash
cmake --build . -j$(nproc)
```

Executable output:

```text
build/SubtitleEditFree
```

---

# Install Application

Make install script executable:

```bash
chmod +x scripts/install.sh
```

Run installer:

```bash
./scripts/install.sh
```

The installer will:

- Copy executable to `/opt/SubtitleEditFree`
- Install application icon
- Create GNOME desktop entry
- Register app in Linux application launcher

After installation, the application will appear in:

- GNOME App Grid
- Activities Search
- Desktop launcher systems

---

# Desktop Entry

Installed desktop file:

```text
/usr/share/applications/SubtitleEditFree.desktop
```

Installed icon:

```text
/usr/share/icons/hicolor/256x256/apps/SubtitleEditFree.png
```

---

# Uninstall

Remove installed files:

```bash
sudo rm -rf /opt/SubtitleEditFree
```

Remove desktop entry:

```bash
sudo rm -f /usr/share/applications/SubtitleEditFree.desktop
```

Remove icon:

```bash
sudo rm -f /usr/share/icons/hicolor/256x256/apps/SubtitleEditFree.png
```

Refresh desktop database:

```bash
sudo update-desktop-database
```

---

# License

MIT License

---

# Author

Trương Hải Đăng  
Email: haidanghth910@gmail.com  
Website: https://truonghaidang.com
