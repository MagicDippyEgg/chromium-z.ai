# Halcyon — Voice & Copy Guide

This is the rulebook for every word the browser says to the user. The brand-level intent is in `brand_book.md §9`. This file is the operational reference.

## The five rules

1. **No exclamation marks.** Ever. Not in chrome. Not in WebUI. Not in error pages. Not in crash messages. The browser is not excited.
2. **No greetings except the NTP greeting.** No "Welcome to Halcyon", no "Welcome back", no "Hi there", no "Hello". The NTP greeting is the single exception, and it is just a time-of-day greeting with the user's name if known.
3. **No marketing copy.** The browser never describes its own features in adjectives. Buttons are verbs; settings rows are noun phrases; descriptions say what the thing does, not how good it is.
4. **Short sentences.** If a sentence has a comma in the middle, see if it can be two sentences.
5. **Plain verbs.** Save, Download, Open, Close. Not "Save it for later", not "Grab this file", not "Get going".

## Word choices

| Use                | Don't use                | Why |
|--------------------|--------------------------|-----|
| Search             | Lookup, look up          | The web's word for it. |
| Reload             | Refresh                  | "Reload" is what the page is doing. |
| Close              | Exit (for tabs), Quit (in chrome) | "Close" for tabs, "Quit Halcyon" only for the whole-app menu item. |
| Bookmark           | Favorite, favourite      | Halcyon bookmarks live in cairns, but the verb is still "bookmark". |
| Private window     | Incognito window         | We are not Google. |
| The Cove           | Incognito, private browsing mode | The Cove is the Halcyon name for it. |
| Settings           | Preferences              | Both words are fine; "Settings" is shorter. |
| Halcyon            | the Halcyon browser, your Halcyon | The product is a proper noun, not "the X browser". |
| Margin note        | Annotation, sticky note  | It's a margin note. |
| Cairn              | Bookmark folder, group   | A cairn is a cairn. |
| Tides              | History                  | The page is called Tides. |
| The Lantern        | Privacy indicator        | It's the Lantern. |
| Quiet Mode         | Do Not Disturb, focus mode | It's Quiet Mode. |
| Quill              | Notes, notepad           | It's Quill. |
| Anchored tab       | Pinned tab, sticky tab   | "Anchored" is the Halcyon word. |

## Sentence patterns

### Status messages

Chrome: *"Your file has been downloaded."*
Halcyon: *"Downloaded."*

Chrome: *"1 file downloaded to ~/Downloads."*
Halcyon: *"1 file downloaded."*

Chrome: *"Halcyon is updating. Don't turn off your computer."*
Halcyon: *"Halcyon is updating. Keep your computer on."*

Chrome: *"Tab crashed. Restore?"*
Halcyon: *"This tab crashed. Reload."*

The pattern: **state + verb.** Don't narrate the file's emotional journey.

### Errors

Chrome: *"Aw, Snap! Something went wrong while displaying this webpage."*
Halcyon: *"This tab crashed."*

Chrome: *"ERR_NAME_NOT_RESOLVED — halcyon.example's server IP address could not be found."*
Halcyon: *"Couldn't find halcyon.example. Check the address or your connection."*

Chrome: *"Your connection is not private. Attackers might be trying to steal your information from example.com (for example, passwords, messages, or credit cards)."*
Halcyon: *"example.com's identity can't be verified. Don't enter passwords or payment details here."*

The pattern: **what happened, in plain words, then what to do (or not do).** No technical error codes in the user-facing string — they go in the "details" expandable, which is collapsed by default.

### Permissions

Chrome: *"example.com wants to: Know your location. [Block] [Allow]"*
Halcyon: *"example.com wants to know your location. [Allow once] [Block]"*

Note: Halcyon's default button order puts "Allow once" before "Block" but **bolds Block**, because Block is the safer default. The user can click either. There is no "Always allow" button by default — that's in the dropdown.

The pattern: **the request in a single short sentence, with the safer default prominent.**

### Empty states

Chrome downloads empty state: *"Downloads are paused. Click to resume."*
Halcyon downloads empty state: *"Nothing here yet."*

Chrome history empty state: *"You have no browsing history."*
Halcyon Tides empty state: *"Your Tides are empty."*

The pattern: **a single, plain sentence. No illustration of an empty box, no "Get started by visiting a website" suggestion.**

### Confirmations

Chrome: *"Are you sure you want to clear all browsing data? This will permanently delete X items."*
Halcyon: *"Clear all Tides? This can't be undone."*

The pattern: **the action as a question, the consequence in one short clause.** No "Are you sure" prefix — the question is the sure.

### The NTP greeting

```text
Good morning, Sarah.
```

OR, if no name is set:

```text
Good morning.
```

The greeting uses the user's local time of day:
- 05:00–11:59: "Good morning"
- 12:00–16:59: "Good afternoon"
- 17:00–21:59: "Good evening"
- 22:00–04:59: "Good evening"  (we don't say "Good night" — night implies sleep, and the user is awake)

It is set in `--halcyon-text-display` (34px Inter Tight 500). It fades in over 600ms.

Below it, smaller (`--halcyon-text-body-lg`, Drift):

```text
Tuesday, the third of November.
```

Date format: `<weekday>, the <ordinal> of <month>`. This is deliberately British-style for English — Halcyon's voice is slightly more formal than Chrome's. For other locales, use the locale's natural long-date format.

### The quote

Below the date, in serif (`--halcyon-text-serif`, italic, 22px), with 24px line-spacing:

```text
"The faculty of voluntarily bringing back
a wandering attention, over and over again,
is the very root of judgment, character, and will."
                                — William James
```

The quote is centered. The attribution is right-aligned, indented to align with the right edge of the quote text. Quotes rotate once per day at local midnight, drawn from a curated set of about 200 quotes about attention, stillness, time, and quiet — see `halcyon_ntp/quotes.json`.

We did not invent quotes. Every quote in the file is real and attributable. The file's header documents the source of each.

### The Cove (private window) greeting

When the user opens a Cove window, the start page shows:

```text
This is the Cove.

Pages you open here won't be saved to your Tides,
and downloads won't be saved to your downloads list.
Your bookmarks and existing settings still work.

When you close this window, what you did here is gone.
```

That's the entire copy. No "incognito man" icon. No "you have exited incognito mode" toast. The Cove is just the part of the beach where the tide doesn't reach, and the copy says so plainly.

### Settings

Setting row titles are noun phrases:
- "Quiet Mode"  (not "Enable Quiet Mode")
- "Default search engine"
- "Block third-party cookies"

Setting row descriptions are sentences, not fragments:
- "Mutes notifications, popups, and autoplay across all tabs."  ✓
- "Mutes notifications, popups, and autoplay."  ✗ (fragment)

Toggle labels: just "On" and "Off", lower-case, in `--halcyon-text-2`.

### About Halcyon

```text
Halcyon
Version 1.0.0

Halcyon is a calm browser built on Chromium.
The engine is Blink; the JavaScript engine is V8.
You're using build 1.0.0, channel stable, 64-bit.

Halcyon is open source. The source code is at
github.com/halcyon-browser/halcyon.

Chromium is © The Chromium Authors and licensed under
the BSD 3-Clause License. Halcyon's modifications are
© The Halcyon Authors and licensed under the same.
```

That's the whole About page. No logos of upstream vendors, no animated splash, no "Made with ♥".

## Punctuation

- **Periods at the end of full sentences.** No periods at the end of fragments.
- **Sentence case for headings and menu items.** Not Title Case. Not ALL CAPS.
- **No Oxford comma.** ("tabs, bookmarks and history", not "tabs, bookmarks, and history".)
- **Apostrophes are curly ('), not straight (').** Same for quotes (" ").
- **En dash (–) for ranges.** "10–20MB", not "10-20MB" or "10 to 20MB".
- **No ellipses (…) in chrome buttons.** "Reload", not "Reload…". (Chrome uses ellipses to indicate "this opens a dialog"; Halcyon considers this noise. If the user clicks Reload, the page reloads; the user doesn't need to be warned.)
- **Ellipses are allowed in user-facing prose** — for example, the empty-state of a search field can be "Type to search…"

## Numbers

- File sizes: binary units (KiB, MiB, GiB), space before unit ("3.4 MiB"), no decimal places above 100 MiB.
- Counts: "1 download", "3 downloads" — never "download(s)".
- Time: "Just now", "5 minutes ago", "1 hour ago", "Yesterday", "2 days ago", "Nov 3", "Nov 3, 2024". Plain.
- Percentages: integer for ≥10%, one decimal place for <10%. "45%", "3.4%".

## Languages other than English

Halcyon's voice is defined for English first. For other locales, the principle is the same — calm, plain, no exclamation marks — but the specific word choices are made by each locale's translator. We do not machine-translate chrome UI copy.

## What we delete

The following upstream strings are removed entirely from Halcyon (not just reworded):

- "Welcome back. Pick up where you left off."
- "Make Chrome your default browser"
- "Get help with Chrome"
- "Send feedback" (we keep "Report a problem with Halcyon", but it's in Settings → About, not the menu)
- "What's new"
- "Continue where you left off" (it just happens)
- "Use Chrome without an account" (Halcyon doesn't prompt for an account)
- "Sync is paused" (Halcyon sync is opt-in only; if off, no banner)
- All onboarding tour strings
- "Open as window" (we use Anchored tabs instead — same UX, no copy)
- "Send to your devices" (removed from the menu)
- "Cast…" (removed from the menu)
- "Create QR code for this page" (removed from the menu; QR codes for sharing are available via the Share sheet on the system, not in Halcyon's chrome)

These removals are done in `chrome/app/generated_resources/generated_resources.grd` and `chrome/app/chromium_strings/chromium_strings.grd` by setting the message text to empty and the `translateable` attribute to `false`. (Setting to empty rather than removing the entry keeps the resource ID stable, which avoids breaking anything that references the ID.)
