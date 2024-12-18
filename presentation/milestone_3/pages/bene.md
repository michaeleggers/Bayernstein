---
layout: center
---

# Gegner KI

---
hideInToc: true
---

# Wie nehmen wir unsere Umgebung wahr?
- Organismen interagieren mit der Welt durch ihre **Sinne**.
- Die Sinne helfen uns 
   - zu überleben
   - uns anzupassen
   - und Entscheidungen zu treffen

---
hideInToc: true
---

# Wie nehmen wir unsere Umgebung wahr?
- Interne und Externe Sinne (z.B. Hunger und Sicht)
- Fun fact: Es gibt Schlangenarten die Infrarot sehen können. <Cite bref="PhysRevLett97" />
- **Frage**: Wie würde unser Leben aussehen wenn wir Infrarot sehen könnten?


---
hideInToc: true
---

![Dune - Giedi Prime](/img/bene/dune-2-austin-butler-feyd-rautha-black-white-explained.avif)

<footer class="absolute bottom-0 left-0 right-0 p-2">
    <small>
        Bildquelle: <a href="https://screenrant.com/dune-2-austin-butler-feyd-rautha-black-white-explained/">
https://screenrant.com/dune-2-austin-butler-feyd-rautha-black-white-explained/
</a>
    </small>
</footer>

---
hideInToc: true
---

# Wie funktioniert Sehen überhaupt?
- Sicht beinhaltet das wahrnehmen und umwandeln von Licht in Information.
- Stäbchen und Zapfen auf der Netzhaut
- Wie modelliert man Sicht aber nun performant?


---
hideInToc: true
---

# Unser Sicht Modell

**Was nimmt die Entity wahr?**
- **Distanz**: Wie weit ist ein Objekt entfernt?  
- Die **Z-Position** des Objekts im Raum.  
- **Winkel**: Relative Ausrichtung des Objekts zur Sichtlinie.


 später noch: **Verdeckung** durch Level Geometrie

---
layout: center
---

# Demo

---
hideInToc: true
---

# Aussicht - Weitere Sinne

- Hören -> Impuls der alles in einem Radius benachrichtigt
- Riechen -> Gerüche in Halflife sind quasi Sounds mit einem Flag
- Fühlen -> physics system mit collision detection


