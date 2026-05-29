#!/bin/bash

# Prüfen, ob das Skript als root ausgeführt wird
if [ "$EUID" -ne 0 ]; then 
  echo "Bitte starte das Skript mit sudo: sudo ./install.sh"
  exit 1
fi

echo "-------------------------------------------------------"
echo "   Pinguin-Installer: Installation wird gestartet...   "
echo "-------------------------------------------------------"

# 1. Abhängigkeiten installieren (Void Linux spezifisch)
echo "[1/4] Installiere benötigte Pakete (Abhängigkeiten)..."
xbps-install -Sy base-devel gtk+3-devel pkg-config pkgconf gparted polkit

if [ $? -ne 0 ]; then
    echo "Fehler beim Installieren der Abhängigkeiten!"
    exit 1
fi

# 2. Den Installer kompilieren
echo "[2/4] Kompiliere Pinguin-Installer..."
if [ -f "./build.sh" ]; then
    chmod +x build.sh
    ./build.sh
else
    echo "Fehler: build.sh nicht gefunden!"
    exit 1
fi

# Prüfen, ob die Kompilierung erfolgreich war
if [ ! -f "./pinguin-installer" ]; then
    echo "Fehler: Die Datei pinguin-installer wurde nicht erstellt!"
    exit 1
fi

# 3. Installation nach /usr/local/bin/
echo "[3/4] Kopiere Programm nach /usr/local/bin/..."
cp ./pinguin-installer /usr/local/bin/pinguin-installer
chmod +x /usr/local/bin/pinguin-installer

# 4. Abschluss
echo "[4/4] Installation abgeschlossen!"
echo "-------------------------------------------------------"
echo "Du kannst den Installer jetzt jederzeit mit dem Befehl:"
echo "      sudo pinguin-installer"
echo "im Terminal starten."
echo "-------------------------------------------------------"
