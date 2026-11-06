/**
 * About Halcyon — chrome://halcyon
 *
 * The page asks the C++ side for the build info (version, channel, build
 * number, arch) and renders it. The only interactions are two buttons:
 *   - Report a problem with Halcyon → opens chrome://settings?topic=report
 *   - Release notes → opens chrome://settings?topic=release-notes
 *
 * Both buttons call chrome.send with their respective message names.
 */

(() => {
  'use strict';

  const versionEl = document.getElementById('halcyon-about-version');
  const buildEl = document.getElementById('halcyon-about-build');
  const channelEl = document.getElementById('halcyon-about-channel');
  const archEl = document.getElementById('halcyon-about-arch');

  const reportBtn = document.getElementById('halcyon-about-report-problem');
  const releaseBtn = document.getElementById('halcyon-about-release-notes');

  window.halcyon = window.halcyon || {};

  /**
   * info = {
   *   version: "1.0.0",
   *   build:   "1.0.0",
   *   channel: "stable",
   *   arch:    "64-bit"
   * }
   */
  window.halcyon.aboutInfo = (info) => {
    if (!info) return;
    if (versionEl) versionEl.textContent = info.version || '1.0.0';
    if (buildEl) buildEl.textContent = info.build || info.version || '1.0.0';
    if (channelEl) channelEl.textContent = info.channel || 'stable';
    if (archEl) archEl.textContent = info.arch || '64-bit';
  };

  document.addEventListener('DOMContentLoaded', () => {
    reportBtn.addEventListener('click', () => {
      chrome.send('halcyonAboutReportProblem', []);
    });
    releaseBtn.addEventListener('click', () => {
      chrome.send('halcyonAboutReleaseNotes', []);
    });
    chrome.send('halcyonAboutRequestInfo', []);
  });
})();
