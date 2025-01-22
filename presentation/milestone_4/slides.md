---
# You can also start simply with 'default'
theme: seriph
colorSchema: dark
canvasWidth: 680
# random image from a curated Unsplash collection by Anthony
# like them? see https://unsplash.com/collections/94734566/slidev
# background: #fff
# some information about your slides (markdown enabled)
title: Games Engineering
titleTemplate: "%s"
info: |
  # Games Engineering
# apply unocss classes to the current slide
class: text-center
defaults: # https://sli.dev/features/drawing

drawings:
  persist: false
# slide transition: https://sli.dev/guide/animations.html#slide-transitions
transition: slide-left
# enable MDC Syntax: https://sli.dev/features/mdc
mdc: true
# take snapshot for each slide in the overview
overviewSnapshots: true
addons:
  - slidev-addon-citations
biblio:
  filename:
    - references.bib
hideInToc: true
---

# Games Engineering
## Quake-Style Shooter from Scratch

<img src="/img/titlescreen.png" style="width: 280px; margin: 0 auto;">

### Meilenstein 4

<!--
The last comment block of each slide will be treated as slide notes. It will be visible and editable in Presenter Mode along with the slide. [Read more in the docs](https://sli.dev/guide/syntax.html#notes)
-->
---
layout: two-cols
---

Bayer, Nico

DePaoli, Fabian

Eggers, Michael

Köhler, Benedikt

Rittenschober, Johann

<br/>
<br/>

Hochschule für angewandte Wissenschaften München

<span>am <Today/></span>

::right::


<div class="flex justify-end">
<div class="w-30">

![Hm Logo](/img/hm-logo.png)

</div>
</div>

---
transition: fade-out
level: 2
---

# Agenda

<Toc maxDepth="1"/>

---
src: ./pages/nico.md
---

---
src: ./pages/fabi.md
---

---
src: ./pages/johann.md
---

---
src: ./pages/bene.md
---

---
hideInToc: true
layout: center
---

# Danke für Eure Aufmerksamkeit!

Nun der Trailer 👀



---
src: ./pages/demo.md
hide: true
---
