---
layout: center
---

# Gegner KI

<!-- This is a **note** -->

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

# Was ist passiert

<video controls class="h-[90%]">
  <source src="../public/img/bene/steering-behaviours.webm" type="video/webm">
  Your browser does not support the video tag.
</video>

---
level: 2
---

# Steering Behaviours

Craig Reynolds beschreibt 3 Layer

- Action Selection
- Steering
- Locomotion

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

# Wie wird die Kraft zu Bewegung

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

![Steering Prämisse](../public/img/bene/05_steering_2.webp)

<footer class="absolute bottom-0 left-0 right-0 p-2">
    <small>
        Bildquelle: <a href="https://natureofcode.com/autonomous-agents/">https://natureofcode.com/autonomous-agents/</a>
    </small>
</footer>

---
level: 2
---

## Steering Force

![Gewünschte Geschwindigkeit](../public/img/bene/05_steering_3.webp)

<footer class="absolute bottom-0 left-0 right-0 p-2">
    <small>
        Bildquelle: <a href="https://natureofcode.com/autonomous-agents/">https://natureofcode.com/autonomous-agents/</a>
    </small>
</footer>

---
level: 2
---

## Steering Force

![Maximale Geschwindigkeit](../public/img/bene/05_steering_4.webp)

<footer class="absolute bottom-0 left-0 right-0 p-2">
    <small>
        Bildquelle: <a href="https://natureofcode.com/autonomous-agents/">https://natureofcode.com/autonomous-agents/</a>
    </small>
</footer>

---
level: 2
---

## Steering Force

![Neue Kraft](../public/img/bene/05_steering_5.webp)

<footer class="absolute bottom-0 left-0 right-0 p-2">
    <small>
        Bildquelle: <a href="https://natureofcode.com/autonomous-agents/">https://natureofcode.com/autonomous-agents/</a>
    </small>
</footer>

---
level: 2
---

## Pfad Verfolgung

![Pfand Verfolgung](../public/img/bene/05_steering_21.webp)

<footer class="absolute bottom-0 left-0 right-0 p-2">
    <small>
        Bildquelle: <a href="https://natureofcode.com/autonomous-agents/">https://natureofcode.com/autonomous-agents/</a>
    </small>
</footer>

---
level: 2
hide: true
---

## Multi Segment Pfad

![Welches Segment ist das Richtige](../public/img/bene/05_steering_33.webp)

<footer class="absolute bottom-0 left-0 right-0 p-2">
    <small>
        Bildquelle: <a href="https://natureofcode.com/autonomous-agents/">https://natureofcode.com/autonomous-agents/</a>
    </small>
</footer>

---
level: 2
---

# Ausblick

- Bug Fixing
- State Machine & Steerign Behaviours zusammen bringen
- weitere State Machines (Verhalten) bauen
