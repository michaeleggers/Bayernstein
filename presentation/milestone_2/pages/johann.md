---
title: Audio
---

# Audio (Library)
<style>
.col-container {
    display: flex;
    width: 100%;
    gap: 15px;
    margin-top: 20px;
}

.col {
    flex: 1;
    background-color: #8883;
    padding: 10px 15px;
    border-radius: 8px;
}

ul {
    list-style-type: "+";
    font-size: 0.9em;
    margin: 10px 0;
    line-height: 1.5em;
}
ul li {
    margin: 0 0 5px 10px;
}

ul li::marker {
    color: green;
    font-weight: bold;
}

ul:last-child {
    list-style-type: "-";
}
ul:last-child li::marker {
    color: red;
}
</style>

<div class="col-container">
    <div class="col">
        <h4>OpenAL</h4>
        <ul>
            <li>scheinbar sehr mächtig</li>
            <li>speziell für 3D Game-Audio designed</li>
        </ul>
        <ul>
            <li>sehr Low-Level, API ähnlich wie OpenGL</li>
            <li>keine direkte 2D-Audio Unterstützung</li>
            <li>keine Einstiegs-Dokumentation vorhanden, nur Spezifikation</li>
        </ul>
    </div>
    <div class="col">
        <h4>miniaudio</h4>
        <ul>
            <li>Feature-Umfang: <div style="opacity:0.6;">2d/3d audio, Effekte per Node-Graph System, Mixing, ...</div></li>
            <li>Low-Level und High-Level API</li>
        </ul>
        <ul>
            <li>Dokumentation (und Code-Snippets) etwas schwer zu überblicken</li>
            <li>Ungenauigkeiten bzgl. Feature-Umfang</li>
        </ul>
    </div>
    <div class="col" style="box-shadow: 0 0 20px #0008">
        <h4><strong>SoLoud</strong></h4>
        <ul>
            <li>Dokumentation (und Code-Snippets) leicht zu verstehen</li>
            <li>Feature-Umfang: <div style="opacity:0.6;">2d/3d audio, Effekte, Mixing, integrierter Synthesizer, ...</div></li>
            <li>miniaudio als Backend enthalten</li>
        </ul>
        <ul>
            <li>ggf. langsamer / weniger optimiert als (lower-level) Alternativen</li>
            <li>evtl. eingeschränkt für weiter fortgeschrittene Use-Cases</li>
        </ul>
    </div>
</div>

---
hideInToc: true
---

# Audio (Implementierung)
<img src="/img/johann/concepts.png" style="right:15px;top:15px;position:absolute;width:calc(50% - 15px);">

Notwendig (Basics):
- 2D Audio für Musik / Ambiente
- 3D Audio für positionierte Emitter (z.B. Gegner)
- Einbindung in System- / Entity-Events  
(z.B. UI Clicks, Schuss-Trigger, Footsteps, etc.)

<br>

Nice to have:
- Spiel-Zustand für Sounds (z.B. Wechsel von 'idle' zu 'combat' Modus)
- Sound-Randomisierung (z.B. mehrere Samples, volume/pitch/effect Moodulationen, etc.)
- Prozedurale Sounds mit dynamischer Länge (z.B. Türen, bewegbare Plattformen, etc.)
- Räumliche Sound-Effekte (Räume, obstruction / occlusion / attenuation, etc.)

<!--
- maybe even music matched effects (e.g. idle to combat transition, timed shots/explosions, etc.), probably needs beat information (bpm, etc. ?) of soundtrack files
-->
