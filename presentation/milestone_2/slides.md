---
# You can also start simply with 'default'
theme: seriph
# random image from a curated Unsplash collection by Anthony
# like them? see https://unsplash.com/collections/94734566/slidev
background: https://cover.sli.dev
# some information about your slides (markdown enabled)
title: Welcome to Slidev
info: |
  ## Slidev Starter Template
  Presentation slides for developers.

  Learn more at S.i.dev](https://sli.dev)
# apply unocss classes to the current slide
class: text-center
# https://sli.dev/features/drawing
drawings:
  persist: false
# slide transition: https://sli.dev/guide/animations.html#slide-transitions
transition: slide-left
# enable MDC Syntax: https://sli.dev/features/mdc
mdc: true
# take snapshot for each slide in the overview
overviewSnapshots: true
---

# Welcome to Slidev

Presentation slides for developers

<div class="pt-12">
  <span @click="$slidev.nav.next" class="px-2 py-1 rounded cursor-pointer" hover="bg-white bg-opacity-10">
    Press Space for next page <carbon:arrow-right class="inline"/>
  </span>
</div>

<div class="abs-br m-6 flex gap-2">
  <button @click="$slidev.nav.openInEditor()" title="Open in Editor" class="text-xl slidev-icon-btn opacity-50 !border-none !hover:text-white">
    <carbon:edit />
  </button>
  <a href="https://github.com/slidevjs/slidev" target="_blank" alt="GitHub" title="Open in GitHub"
    class="text-xl slidev-icon-btn opacity-50 !border-none !hover:text-white">
    <carbon-logo-github />
  </a>
</div>

<!--
The last comment block of each slide will be treated as slide notes. It will be visible and editable in Presenter Mode along with the slide. [Read more in the docs](https://sli.dev/guide/syntax.html#notes)
-->

---
transition: fade-out
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

---

# Quellenangaben
1. Mat Buckland. Programming Game AI by Example. en. Jones &
Bartlett Learning, 2005. isbn: 9781556220784.
2. Kasper Fauerby. “Improved Collision detection and Response”.
In: Unpublished manuscript (2003). url:
https://www.peroxide.dk/papers/collision/collision.pdf.
3. J. Gregory. Game Engine Architecture, Third Edition. CRC Press, 2018. isbn: 9781351974288. url: https://books.google.de/books?id=1g1mDwAAQBAJ.
4. Andrew Kirmse. Game Programming Gems 4 (Game Programming Gems Series). USA: Charles River Media, Inc., 2004.
isbn: 1584502959.
5. Jeff Linahan. “Improving the Numerical Robustness of Sphere Swept Collision Detection”. In: ArXiv (Okt. 2012). url:
https://www.semanticscholar.org/paper/Improving-the-Numerical-Robustness-of-Sphere-Swept-Linahan/0e551998089fc84bf86f89ea07193377b7a12f6f (besucht am26. 10. 2024).

---

# Quellenangaben
6. Paul Nettle. “Generic Collision Detection for Games Using Ellipsoids”. In: Unpublished manuscript (2000). url:
http://www.fluidstudios.com/publications.html.
7. Craig W. Reynolds. “Flocks, Herds, and Schools: A Distributed Behavioral Model”. In: Siggraph (1987). url: https://www.red3d.com/cwr/papers/1987/SIGGRAPH87.pdf.
8. timbergeron. QSS-M. url: https://github.com/timbergeron/QSS-M (besucht am 29. 10. 2024).
9. Daniel Shiffman.  “Nature of Code”, 2024. isbn: 978-1-7185-0371-7 (ebook)

---
src: ./pages/demo.md
hide: true
---
