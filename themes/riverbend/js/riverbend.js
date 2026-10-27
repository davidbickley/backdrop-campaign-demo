/**
 * @file
 * Lines the hero tally's bottom edge up with the hero buttons.
 *
 * The copy column is vertically centred, so the buttons' position depends on
 * the hero's height and the text length; CSS alone can't pin a box in the
 * other column to it. Without this script the tally sits at its CSS default.
 */
(function () {
  'use strict';

  function align() {
    var hero = document.querySelector('.rb-hero');
    if (!hero) {
      return;
    }
    var tally = hero.querySelector('.rb-hero-art .rb-tally');
    var actions = hero.querySelector('.rb-actions');
    if (!tally || !actions) {
      return;
    }
    tally.style.bottom = '';
    // Only when the tally overlays the photo (two-column layout).
    if (window.getComputedStyle(tally).position !== 'absolute') {
      return;
    }
    var art = tally.parentNode.getBoundingClientRect();
    tally.style.bottom = Math.max(0, art.bottom - actions.getBoundingClientRect().bottom) + 'px';
  }

  document.addEventListener('DOMContentLoaded', align);
  window.addEventListener('load', align);
  window.addEventListener('resize', align);
  if (document.fonts && document.fonts.ready) {
    document.fonts.ready.then(align);
  }
})();
