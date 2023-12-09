<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="de_DE" sourcelanguage="en_US">
<context>
    <name></name>
    <message id="cutelystd-cli-desc">
        <source>Fast, developer-friendly server.</source>
        <oldsource>Fast, developer-friendliy server.</oldsource>
        <extracomment>CLI app description</extracomment>
        <translation>Schneller, entwicklerfreundlicher Server.</translation>
    </message>
    <message id="cutelystd-opt-ini-desc">
        <source>Load config from INI file. When used multiple times, content will be merged and same keys in the sections will be overwritten by content from later files.</source>
        <oldsource>Load config from ini file.</oldsource>
        <extracomment>CLI option description</extracomment>
        <translation>Lade Konfiguration aus INI-Datei. Wenn mehrfach angegeben, werden die Werte gleicher Schlüssel in den Sektionen durch Werte aus später angegebenen Dateien überschrieben.</translation>
    </message>
    <message id="cutelystd-opt-value-file">
        <source>file</source>
        <extracomment>CLI option value name</extracomment>
        <translation>Datei</translation>
    </message>
    <message id="cutelystd-opt-json-desc">
        <source>Load config from JSON file. When used multiple times, content will be merged and same keys in the sections will be overwritten by content from later files.</source>
        <oldsource>Load config from JSON file.</oldsource>
        <extracomment>CLI option description</extracomment>
        <translation>Lade Konfiguration aus JSON-Datei. Wenn mehrfach angegeben, werden die Werte gleicher Schlüssel in den Sektionen durch Werte aus später angegebenen Dateien überschrieben.</translation>
    </message>
    <message id="cutelystd-opt-chdir-desc">
        <source>Change to the specified directory before the application is loaded.</source>
        <extracomment>CLI option description</extracomment>
        <translation>Wechsle in das angegebene Verzeichnis bevor die Anwendung geladen wird.</translation>
    </message>
    <message id="cutelystd-opt-value-directory">
        <source>directory</source>
        <extracomment>CLI option value name</extracomment>
        <translation>Verzeichnis</translation>
    </message>
    <message id="cutelystd-opt-chdir2-desc">
        <source>Change to the specified directory after the application has been loaded.</source>
        <extracomment>CLI option description</extracomment>
        <translation>Wechsle in das angegebene Verzeichnis nachdem die Anwendung geladen wurde.</translation>
    </message>
    <message id="cutelystd-opt-lazy-desc">
        <source>Use lazy mode (load the application in the workers instead of master).</source>
        <extracomment>CLI option description</extracomment>
        <translation>Nutze den Lazy-Modus (lädt die Anwendung in den Workern anstatt im Master).</translation>
    </message>
    <message id="cutelystd-opt-application-desc">
        <source>Path to the application file to load.</source>
        <oldsource>The Application to load.</oldsource>
        <extracomment>CLI option description</extracomment>
        <translation>Pfad zur zu ladenden Anwendungsdatei.</translation>
    </message>
    <message id="cutelystd-opt-threads-desc">
        <source>The number of threads to use. If set to “auto”, the ideal thread count is used.</source>
        <oldsource>The number of threads to use.</oldsource>
        <extracomment>CLI option description</extracomment>
        <translation>Die Anzahl der zu nutzenden Threads. Sie können „auto“ angeben, um automatisch die ideale Anzahl zu nutzen.</translation>
    </message>
    <message id="cutelystd-opt-threads-value">
        <source>threads</source>
        <extracomment>CLI option value name</extracomment>
        <translation>Threads</translation>
    </message>
    <message id="cutelystd-opt-processes-desc">
        <source>Spawn the specified number of processes. If set to “auto”, the ideal process count is used.</source>
        <oldsource>Spawn the specified number of processes.</oldsource>
        <extracomment>CLI option description</extracomment>
        <translation>Erzeuge die angegebene Anzahl von Prozessen. Sie können „auto“ angeben, um automatisch die ideale Anzahl zu nutzen.</translation>
    </message>
    <message id="cutelystd-opt-processes-value">
        <source>processes</source>
        <extracomment>CLI option value name</extracomment>
        <translation>Prozesse</translation>
    </message>
    <message id="cutelystd-opt-master-desc">
        <source>Enable master process.</source>
        <extracomment>CLI option description</extracomment>
        <translation>Master-Prozess aktivieren.</translation>
    </message>
    <message id="cutelystd-opt-listen-desc">
        <source>Set the socket listen queue size. Default value: 100.</source>
        <oldsource>Set the socket listen queue size.</oldsource>
        <extracomment>CLI option description</extracomment>
        <translation>Legt die Größe der Socket-Warteschlange fest. Standardwert: 100.</translation>
    </message>
    <message id="cutelystd-opt-value-size">
        <source>size</source>
        <extracomment>CLI option value name</extracomment>
        <translation>Größe</translation>
    </message>
    <message id="cutelystd-opt-buffer-size-desc">
        <source>Set the internal buffer size. Default value: 4096.</source>
        <oldsource>Set the internal buffer size.</oldsource>
        <extracomment>CLI option description</extracomment>
        <translation>Legt die Größe des internen Zwischenspeichers fest. Standardwert: 4096.</translation>
    </message>
    <message id="cutelystd-opt-value-bytes">
        <source>bytes</source>
        <extracomment>CLI option value name</extracomment>
        <translation>Bytes</translation>
    </message>
    <message id="cutelystd-opt-post-buffering-desc">
        <source>Sets the size after which buffering takes place on the hard disk instead of in the main memory. Default value: -1.</source>
        <oldsource>Sets the size after which buffering takes place on the hard disk instead of in the main memory.</oldsource>
        <extracomment>CLI option description</extracomment>
        <translation>Legt die Größe fest, ab der die Pufferung auf der Festplatte statt im Hauptspeicher erfolgt. Standardwert: -1.</translation>
    </message>
    <message id="cutelystd-opt-post-buffering-bufsize-desc">
        <source>Set the buffer size for read() in post buffering mode. Default value: 4096.</source>
        <oldsource>Set the buffer size for read() in post buffering mode.</oldsource>
        <extracomment>CLI option description</extracomment>
        <translation>Legt die Puffergröße für read() im Post-Buffering-Modus fest. Standardwert: 4096.</translation>
    </message>
    <message id="cutelystd-opt-http-socket-desc">
        <source>Bind to the specified TCP socket using the HTTP protocol.</source>
        <extracomment>CLI option description</extracomment>
        <translation>Bindung an den angegebenen TCP-Socket unter Verwendung des HTTP-Protokolls.</translation>
    </message>
    <message id="cutelystd-opt-value-address">
        <source>address</source>
        <extracomment>CLI option value name</extracomment>
        <translation>Adresse</translation>
    </message>
    <message id="cutelystd-opt-http2-socket-desc">
        <source>Bind to the specified TCP socket using the HTTP/2 protocol.</source>
        <extracomment>CLI option description</extracomment>
        <translation>Bindung an den angegebenen TCP-Socket unter Verwendung des HTTP/2-Protokolls.</translation>
    </message>
    <message id="cutelystd-opt-http2-header-table-size-desc">
        <source>Sets the HTTP/2 header table size.</source>
        <extracomment>CLI option description</extracomment>
        <translation>Legt die Größe der Tabelle für die HTTP/2 Header-Daten fest.</translation>
    </message>
    <message id="cutelystd-opt-upgrade-h2c-desc">
        <source>Upgrades HTTP/1 to H2c (HTTP/2 Clear Text).</source>
        <oldsource>Upgrades HTTP/1 to H2c (HTTP/2 Clear Texte).</oldsource>
        <extracomment>CLI option description</extracomment>
        <translation>Wertet HTTP/1 zu H2c (HTTP/2 Clear Text) auf.</translation>
    </message>
    <message id="cutelystd-opt-https-h2-desc">
        <source>Negotiate HTTP/2 on HTTPS socket.</source>
        <extracomment>CLI option description</extracomment>
        <translation>Handelt HTTP/2 auf einem HTTPS-Socket aus.</translation>
    </message>
    <message id="cutelystd-opt-https-socket-desc">
        <source>Bind to the specified TCP socket using HTTPS protocol.</source>
        <extracomment>CLI option description</extracomment>
        <translation>Bindung an den angegebenen TCP-Socket unter Verwendung des HTTPS-Protokolls.</translation>
    </message>
    <message id="cutelystd-opt-fastcgi-socket-desc">
        <source>Bind to the specified UNIX/TCP socket using FastCGI protocol.</source>
        <extracomment>CLI option description</extracomment>
        <translation>Bindung an den angegebenen UNIX/TCP-Socket unter Verwendung des FastCGI-Protokolls.</translation>
    </message>
    <message id="cutelystd-opt-socket-access-desc">
        <source>Set the LOCAL socket access, such as &apos;ugo&apos; standing for User, Group, Other access.</source>
        <extracomment>CLI option description</extracomment>
        <translation>Legt den Zugriff auf den LOKALEN Socket fest, wie bspw. &apos;ugo&apos;, was für User, Group und Other steht.</translation>
    </message>
    <message id="cutelystd-opt-socket-access-value">
        <source>options</source>
        <extracomment>CLI option value name</extracomment>
        <translation>Optionen</translation>
    </message>
    <message id="cutelystd-opt-socket-timeout-desc">
        <source>Set internal socket timeouts. Default value: 4.</source>
        <oldsource>Set internal socket timeouts.</oldsource>
        <extracomment>CLI option description</extracomment>
        <translation>Legt interne Zeitfristen für Sockets fest. Standardwert: 4.</translation>
    </message>
    <message id="cutelystd-opt-socket-timeout-value">
        <source>seconds</source>
        <extracomment>CLI option value name</extracomment>
        <translation>Sekunden</translation>
    </message>
    <message id="cutelystd-opt-static-map-desc">
        <source>Map mountpoint to local directory to serve static files. The mountpoint will be removed from the request path and the rest will be appended to the local path to find the file to serve. Can be used multiple times.</source>
        <oldsource>Map mountpoint to static directory (or file).</oldsource>
        <extracomment>CLI option description</extracomment>
        <translation>Einhängepunkt einem lokalen Ordner zuordnen um statische Dateien auszuliefern. Der Einhängepunkt wird vom Anfragepfad entfernt und der Rest wird dem lokalen Pfad angehängt, um die auszuliefernde Datei lokal zu finden. Kann mehrfach angegeben werden.</translation>
    </message>
    <message id="cutelystd-opt-value-static-map">
        <source>/mountpoint=/path</source>
        <oldsource>mountpoint=path</oldsource>
        <extracomment>CLI option value name</extracomment>
        <translation>/Einhängepunkt=/Pfad</translation>
    </message>
    <message id="cutelystd-opt-static-map2-desc">
        <source>Like static-map but completely appending the request path to the local path. Can be used multiple times.</source>
        <oldsource>Like static-map but completely appending the requested resource to the docroot.</oldsource>
        <extracomment>CLI option description</extracomment>
        <translation>Wie static-map, hängt den den Anfragepfad aber komplett an den lokalen Pfad an.  Kann mehrfach angegeben werden.</translation>
    </message>
    <message id="cutelystd-opt-auto-restart-desc">
        <source>Auto restarts when the application file changes.</source>
        <extracomment>CLI option description</extracomment>
        <translation>Startet automatisch neu sobald sich die Anwendungsdatei ändert.</translation>
    </message>
    <message id="cutelystd-opt-touch-reload-desc">
        <source>Reload the application if the specified file is modified/touched.</source>
        <extracomment>CLI option description</extracomment>
        <translation>Startet die Anwendung neu wenn sich die angegebene Datei ändert.</translation>
    </message>
    <message id="cutelystd-opt-tcp-nodelay-desc">
        <source>Enable TCP NODELAY on each request.</source>
        <extracomment>CLI option description</extracomment>
        <translation>Aktiviere bei jeder Anfrage TCP NODELAY.</translation>
    </message>
    <message id="cutelystd-opt-so-keepalive-desc">
        <source>Enable TCP KEEPALIVEs.</source>
        <extracomment>CLI option description</extracomment>
        <translation>Aktiviere TCP KEEPALIVEs.</translation>
    </message>
    <message id="cutelystd-opt-socket-sndbuf-desc">
        <source>Sets the socket send buffer size in bytes at the OS level. This maps to the SO_SNDBUF socket option. Default value: -1.</source>
        <oldsource>Sets the socket send buffer size in bytes at the OS level. This maps to the SO_SNDBUF socket option.</oldsource>
        <extracomment>CLI option description</extracomment>
        <translation>Setzt die Größe des Socket-Sendepuffers in Bytes auf OS-Ebene. Dies entspricht der Socket-Option SO_SNDBUF. Standardwert: -1.</translation>
    </message>
    <message id="cutelystd-opt-socket-rcvbuf-desc">
        <source>Sets the socket receive buffer size in bytes at the OS level. This maps to the SO_RCVBUF socket option. Default value: -1.</source>
        <oldsource>Sets the socket receive buffer size in bytes at the OS level. This maps to the SO_RCVBUF socket option.</oldsource>
        <extracomment>CLI option description</extracomment>
        <translation>Setzt die Größe des Socket-Empfangspuffers in Bytes auf OS-Ebene. Dies entspricht der Socket-Option SO_RCVBUF. Standardwert: -1.</translation>
    </message>
    <message id="cutelystd-opt-websocket-max-size-desc">
        <source>Sets the websocket receive buffer size in kibibytes at the OS level. This maps to the SO_RCVBUF socket option. Default value: 1024.</source>
        <oldsource>Sets the websocket receive buffer size in kibibytes at the OS level. This maps to the SO_RCVBUF socket option.</oldsource>
        <extracomment>CLI option description</extracomment>
        <translation>Setzt die Größe des Websocket-Empfangspuffers in Kibibytes auf OS-Ebene. Dies entspricht der Socket-Option SO_RCVBUF. Standardwert: 1024.</translation>
    </message>
    <message id="cutelystd-opt-websocket-max-size-value">
        <source>kibibyte</source>
        <oldsource>kilobyte</oldsource>
        <extracomment>CLI option value name</extracomment>
        <translation>Kibibyte</translation>
    </message>
    <message id="cutelystd-opt-pidfile-desc">
        <source>Create pidfile (before privilege drop).</source>
        <extracomment>CLI option description</extracomment>
        <translation>Erstelle PID-Datei (vor dem Ändern von Benutzer und Gruppe).</translation>
    </message>
    <message id="cutelystd-opt-value-pidfile">
        <source>pidfile</source>
        <extracomment>CLI option value name</extracomment>
        <translation>PID-Datei</translation>
    </message>
    <message id="cutelystd-opt-pidfile2-desc">
        <source>Create pidfile (after privilege drop).</source>
        <extracomment>CLI option description</extracomment>
        <translation>Erstelle PID-Datei (nach dem Ändern von Benutzer und Gruppe).</translation>
    </message>
    <message id="cutelystd-opt-stop-desc">
        <source>Stop an instance identified by the PID in the pidfile.</source>
        <extracomment>CLI option description</extracomment>
        <translation>Stoppe eine durch die PID in der PID-Datei identifizierte Instanz.</translation>
    </message>
    <message id="cutelystd-opt-uid-desc">
        <source>Setuid to the specified user/uid.</source>
        <oldsource>Setuid to the specified user/uid.
</oldsource>
        <extracomment>CLI option description</extracomment>
        <translation>Führe die Anwendung unter dem angegebenen Benutzer/ID aus.</translation>
    </message>
    <message id="cutelystd-opt-uid-value">
        <source>user/uid</source>
        <extracomment>CLI option value name</extracomment>
        <translation>Benutzer/uid</translation>
    </message>
    <message id="cutelystd-opt-gid-desc">
        <source>Setuid to the specified group/gid.</source>
        <oldsource>Setuid to the specified group/gid.
</oldsource>
        <extracomment>CLI option description</extracomment>
        <translation>Führe die Anwendung unter der angegebenen Grupp/ID aus.</translation>
    </message>
    <message id="cutelystd-opt-gid-value">
        <source>group/gid</source>
        <extracomment>CLI option value name</extracomment>
        <translation>Gruppe/gid</translation>
    </message>
    <message id="cutelystd-opt-no-init-groups-desc">
        <source>Disable additional groups set via initgroups().</source>
        <extracomment>CLI option description</extracomment>
        <translation>Deaktiviere zusätliche Gruppen die über initgroups() gesetzt werden.</translation>
    </message>
    <message id="cutelystd-opt-chown-socket-desc">
        <source>Change the ownership of the UNIX socket.</source>
        <extracomment>CLI option description</extracomment>
        <translation>Ändere den Eigentümer des UNIX-Socket.</translation>
    </message>
    <message id="cutelystd-opt-chown-socket-value">
        <source>uid:gid</source>
        <extracomment>CLI option value name</extracomment>
        <translation>uid:gid</translation>
    </message>
    <message id="cutelystd-opt-umask-desc">
        <source>Set file mode creation mask.</source>
        <extracomment>CLI option description</extracomment>
        <translation>Lege die Maske zum Erstellen neuer Dateien fest.</translation>
    </message>
    <message id="cutelystd-opt-umask-value">
        <source>mode</source>
        <extracomment>CLI option value name</extracomment>
        <translation>umask</translation>
    </message>
    <message id="cutelystd-opt-cpu-affinity-desc">
        <source>Set CPU affinity with the number of CPUs available for each worker core.</source>
        <extracomment>CLI option description</extracomment>
        <translation type="unfinished"></translation>
    </message>
    <message id="cutelystd-opt-cpu-affinity-value">
        <source>core count</source>
        <extracomment>CLI option value name</extracomment>
        <translation type="unfinished"></translation>
    </message>
    <message id="cutelystd-opt-reuse-port-desc">
        <source>Enable SO_REUSEPORT flag on socket (Linux 3.9+).</source>
        <oldsource>Enable SO_REUSEPORT flag on socket (Linux 3.0+).</oldsource>
        <extracomment>CLI option description</extracomment>
        <translation>Aktiviere die Markierung SO_REUSEPORT auf dem Socket (Linux 3.9+).</translation>
    </message>
    <message id="cutelystd-opt-experimental-thread-balancer-desc">
        <source>Balances new connections to threads using round-robin.</source>
        <extracomment>CLI option description</extracomment>
        <translation>Verteilt neue Verbindungen gleichmäßig auf den Threads nach dem Round-Robin-Verfahren.</translation>
    </message>
    <message id="cutelystd-opt-using-frontend-proxy-desc">
        <source>Enable frontend (reverse-)proxy support.</source>
        <oldsource>Enable frontend (reverse-)prox support.</oldsource>
        <extracomment>CLI option description</extracomment>
        <translation>Aktiviere Unterstützung für Reverse Proxy.</translation>
    </message>
    <message id="cutelystd-err-no-socket-opened">
        <source>No specified sockets were able to be opened</source>
        <translation>Keiner der angegebenen Socket konnte geöffnet werden</translation>
    </message>
    <message id="cutelystd-err-write-pidfile">
        <source>Failed to write pidfile %1</source>
        <translation>Konnte PID-Datei %1 nicht schreiben</translation>
    </message>
    <message id="cutelystd-err-open-local-socket">
        <source>Error on opening local sockets</source>
        <translation>Fehler beim Öffnen des lokalen Sockets</translation>
    </message>
    <message id="cutelystd-err-setgiduid">
        <source>Error on setting GID or UID</source>
        <translation>Fehler beim Setzen von GID oder UID</translation>
    </message>
    <message id="cutelystd-err-no-socket-specified">
        <source>No socket specified</source>
        <translation>Kein Socket angegeben</translation>
    </message>
    <message id="cutelystd-err-fail-setup-app">
        <source>Failed to setup Application</source>
        <translation>Konnte die Anwendung nicht einrichten</translation>
    </message>
    <message id="cutelystd-err-server-not-fully-stopped">
        <source>Server not fully stopped.</source>
        <translation>Der Server wurde nicht vollständig gestoppt.</translation>
    </message>
</context>
</TS>
