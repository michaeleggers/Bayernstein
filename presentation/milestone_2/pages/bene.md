---
layout: center
---

# Gegner KI


---
level: 2
---

# Gegner KI

Was ist passiert?

- Random Walk
- Seek, Flee, Arrive
- Follow Path, Follow Waypoints


---
level: 2
---

# Das ist passiert

<video controls class="h-[90%]">
  <source src="/img/bene/steering-behaviours.webm" type="video/webm">
  Your browser does not support the video tag.
</video>

<!-- 
- seek 
- flee
- arrive
- follow waypoints
    -> folgt nur punkten, ohne pfad dazwischen
- waypoints & path
    -> versucht zurück zum pfad 
 -->
---
level: 2
---

# Steering Behaviours

Craig Reynolds beschreibt 3 Layer [@Reynolds1987]

- Action Selection
- Steering
- Locomotion

<!--
- action selection -> state machine
- steering behaviours
- locomotion -> wie man von a nach b kommt. vgl auto vs laufen

-->
---
level: 2
---

# Steering Force

Vereinfachtes Model um Steering Force zu berechnen:

<div class="flex justify-center">
<div>

$\mathbf{steeringForce} = \mathbf{desiredVelocity} - \mathbf{currentVelocity}$
</div>
</div>

---
level: 2
---

# Wie wird Kraft zu Bewegung

2. Newtonsches Gesetz wird abgeleitet zu:

<div class="flex justify-center">
<div>

$\bf{f} = m\bf{a}$

$\bf{a} = \frac{\bf{f}}{m}$

$\rightarrow \bf{v_{new}} = \bf{v_{old}} + \bf{a}$

</div>
</div>

---
level: 2
---

## Steering Force

<v-switch>
<template #0>

![Steering Prämisse](/img/bene/05_steering_2.webp)

</template>
<template #1>

![Gewünschte Geschwindigkeit](/img/bene/05_steering_3.webp)

</template>

<template #2>

![Maximale Geschwindigkeit](/img/bene/05_steering_4.webp)

</template>


<template #3>

![Neue Kraft](/img/bene/05_steering_5.webp)

</template>
</v-switch>


<footer class="absolute bottom-0 left-0 right-0 p-2">
    <small>
        Bildquelle: <a href="https://natureofcode.com/autonomous-agents/">https://natureofcode.com/autonomous-agents/</a>[@shiffman24]
    </small>
</footer>


---
level: 2
---

## Pfad Verfolgung

![Pfand Verfolgung](/img/bene/05_steering_21.webp)

<footer class="absolute bottom-0 left-0 right-0 p-2">
    <small>
        Bildquelle: <a href="https://natureofcode.com/autonomous-agents/">https://natureofcode.com/autonomous-agents/</a>
    </small>
</footer>

<!--
- aktuelle richtung + offset
- projektion auf pfad
- segment start + projektion + offset
-->

---
level: 2
hide: true
---

## Multi Segment Pfad

![Welches Segment ist das Richtige](/img/bene/05_steering_33.webp)

<footer class="absolute bottom-0 left-0 right-0 p-2">
    <small>
        Bildquelle: <a href="https://natureofcode.com/autonomous-agents/">https://natureofcode.com/autonomous-agents/</a>
    </small>
</footer>

---
level: 2
---

# Ausblick Gegner KI

- Bug Fixing
- State Machine & Steerign Behaviours zusammen bringen
- weitere State Machines (Verhalten) bauen
