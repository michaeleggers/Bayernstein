---
# You can also start simply with 'default'
theme: seriph
# random image from a curated Unsplash collection by Anthony
# like them? see https://unsplash.com/collections/94734566/slidev
background: white
# some information about your slides (markdown enabled)
title: Games Engineering
titleTemplate: "%s"
info: |
  # Games Engineering
# apply unocss classes to the current slide
class: text-center
defaults:
  zoom: 1.3
# https://sli.dev/features/drawing
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
---

# Welcome to Slidev

<!--
The last comment block of each slide will be treated as slide notes. It will be visible and editable in Presenter Mode along with the slide. [Read more in the docs](https://sli.dev/guide/syntax.html#notes)
-->

---
transition: fade-out
level: 2
---

# What is Slidev?

<Toc maxDepth="1"/>

---
src: ./pages/fabi.md
---

---
src: ./pages/johann.md
---

---
src: ./pages/nico.md
---

---
src: ./pages/bene.md
---

---
src: ./pages/michi.md
---

---

# Ausblick

test zitat:
[@Simpson]

---

# Quellenangaben

<BiblioList />

---
src: ./pages/demo.md
hide: true
---
