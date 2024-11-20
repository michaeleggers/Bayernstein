
<style>
.centered-text {
  display: flex;
  justify-content: center;
  align-items: center;
  height: 100vh; /* Volle Höhe des Viewports */
  font-size: 4rem; /* Große Schrift */
  font-weight: bold; /* Fettschrift */
  text-align: center;
}
</style>

<div class="centered-text">
  Demo aktueller Stand
</div>

---

# Trenchbroom Ansicht
<style>
.singlebox {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  border: 1px solid #ddd;
  padding: 0px;
  border-radius: 8px;
  box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
  width: 100%; /* Erhöhte Breite der Box */
  margin-left: auto; /* Box rechts ausrichten */
  margin-right: auto; /* Zentriert die Box */
  height: 450px; /* Einheitliche Höhe */
}

.singlebox img {
  width: 100%; /* Bild füllt die Breite der Box */
  height: auto;
  max-height: 90%; /* Mehr Platz für größere Bilder */
  object-fit: contain; /* Verhindert Beschneiden */
  margin-bottom: 0px;
}
</style>

<div class="singlebox">
  <img src="/img/nico/trenchbroom.png" alt="Trenchbroom" />
  <p>Trenchbroom Ansicht</p>
</div>

---

# Trim Sheets für die Texturen
<style>
.minibox {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  border: 1px solid #ddd;
  padding: 8px;
  border-radius: 8px;
  box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
  height: 200px; /* Einheitliche Höhe der Box */
}

.minibox img {
  width: 100%;
  height: auto; /* Automatische Höhe, um das Seitenverhältnis zu wahren */
  max-height: 80%; /* Begrenzung der Bildhöhe innerhalb der Box */
  object-fit: contain; /* Verhindert Beschneiden und behält Proportionen */
  margin-bottom: 0px;
  margin-top: 10px;
}

.grid {
  display: grid !important;
  grid-template-columns: repeat(2, 1fr) !important; /* 2 Spalten */
  gap: 16px; /* Abstand zwischen den Elementen */
  width: 100%; /* Volle Breite des Containers */
}
</style>
<div class="grid">
  <div class="minibox">
    <img src="/img/nico/boden-edelstein.png" alt="Boden Edelstein" />
    <p>Boden Edelstein</p>
  </div>
  <div class="minibox">
    <img src="/img/nico/trim-sheet-standard.png" alt="Trim Sheet Standard" />
    <p>Trim Sheet Standard</p>
  </div>
  <div class="minibox">
    <img src="/img/nico/trim-sheet-stein.png" alt="Trim Sheet Stein" />
    <p>Trim Sheet Stein</p>
  </div>
  <div class="minibox">
    <img src="/img/nico/trim-sheet-stone.png" alt="Trim Sheet Stone" />
    <p>Trim Sheet Stone</p>
  </div>
</div>
