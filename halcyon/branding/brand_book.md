# Halcyon — Brand Book

> *Calm the web.*

---

## 1. What Halcyon is

Halcyon is a web browser built on a single, deliberate principle: **a browser should protect the user's attention, not compete for it.**

Most modern browsers — including the one Halcyon is built on top of — have gradually become busier, brighter, and more attention-hungry. Promotional tiles on the new tab page. Notification badges on every surface. Suggestions, prompts, "what's new" tour cards, sign-in nags, address-bar experiments. Each is small; together they turn a tool for reading the web into a machine for interrupting the person using it.

Halcyon reverses this. It treats attention as the most valuable thing the user brings to the screen, and treats the browser chrome — the toolbar, the tab strip, the menus, the start page — as something that should *recede* when the user is doing something, and only surface when invited.

It is not minimal for minimalism's sake. It is quiet on purpose, and on a specific purpose.

## 2. The name

**Halcyon** (pronounced *HAL-see-un*):

- In Greek mythology, the halcyon is a bird identified with the kingfisher. It was said to nest on the sea in winter, and for fourteen days — the *halcyon days* — the winds would fall calm and the water would still so the bird could lay its eggs.
- In English, *halcyon* means *calm, peaceful, happy, golden* — most often used of a past time or a state of mind, "the halcyon days."

The name carries exactly what the browser is for: **calm in the middle of a thing that is normally stormy.** The web is a rough sea. Halcyon is the bird that nests on it anyway.

The word is short, easy to type, easy to say, has no existing major browser named after it, and reads as both modern and timeless.

## 3. What the name is *not*

- Not a weather term or a forecast.
- Not a kingfisher logo. (We deliberately did *not* use a bird as the brand mark — see §5.)
- Not a synonym for "slow." Halcyon is fast. Calm and fast are not opposites; busy and fast are.

## 4. Brand promise

> When you open Halcyon, the web gets quieter.
> Not smaller. Not slower. Not less capable.
> Just quieter — so you can hear yourself think.

This promise is the test for every feature, every animation, every default, every string of copy that goes into the product. If a candidate change makes the browser louder, it has to earn its way in by being genuinely more useful. Most things that make a browser louder are not.

## 5. Brand mark

The Halcyon mark is **a single horizontal line resting on a still horizon** — a soft, level rule drawn through a circle, suggesting both calm water and a setting (or rising) sun half-submerged.

It is *not* a bird, deliberately. A bird mark would be too literal, too decorative, and would compete with the rest of the visual language, which is geometric and quiet.

The mark:

```
        ╭───────────────╮
        │               │
        │   ─────────    │   ← the line is the mark
        │               │
        ╰───────────────╯
```

- A circle of even stroke width.
- A single horizontal line through its vertical center, slightly thicker than the circle's stroke.
- The line does not touch the circle. There is a small, even gap on both sides.
- The mark is rendered in the accent color (see §7) on the surface color.
- It scales cleanly from 16px (favicon) to 1024px (about-page hero).

The mark *is allowed* to be reduced to just the horizontal line when the circle would be visually redundant (e.g., as a loading spinner that simply pulses a horizontal line).

## 6. Logo lockup

```
   ───         H A L C Y O N
```

- The mark sits to the left of the wordmark, vertically centered on the cap height.
- The wordmark is set in **Inter Tight, weight 500, tracking +40**, all caps, in the primary text color.
- The mark's circle has the same cap height as the wordmark.
- The mark's horizontal line aligns to the wordmark's vertical center.
- Minimum clear space: the height of the mark on all sides.
- Minimum size: 24px wordmark height. Below that, use the mark alone.

The wordmark is set in caps because lower-case *halcyon* in a sans-serif reads as a generic word; the caps give it a name-as-product feel without shouting.

## 7. Color

The Halcyon palette is built around **a warm off-white paper color and a single muted sage green**, with quiet slate text and a restrained set of supporting tones. The palette is deliberately desaturated. Halcyon does not use a bright, attention-grabbing brand color.

### Light theme

| Role              | Name        | Hex        | Notes |
|-------------------|-------------|------------|-------|
| Page background   | Vellum      | `#FAF8F3`  | Warm off-white. The default canvas. |
| Surface           | Mist        | `#F0EEE8`  | Elevated surfaces: toolbar, tab backgrounds, cards. |
| Surface raised    | Linen       | `#E8E5DC`  | Hover and active surface states. |
| Border            | Thready     | `#D8D4C8`  | Hairline borders, dividers. |
| Primary text      | Haze        | `#2B2F38`  | Body text. Not pure black — soft slate. |
| Secondary text    | Drift       | `#6B6F78`  | Captions, metadata, less important text. |
| Disabled text     | Pale        | `#A8A8A0`  | Disabled states. |
| Accent            | Sage        | `#7A9B76`  | The brand accent. Active states, focused inputs, the mark. |
| Accent deep       | Pine        | `#5F7D5C`  | Pressed, selected-with-keyboard. |
| Link / active     | Tide        | `#5B7C99`  | Hyperlinks, active tab indicator. |
| Caution           | Ember       | `#C4855C`  | Warnings, but never red. (Halcyon avoids red unless the system requires it.) |
| Danger            | Coral       | `#B05945`  | Reserved for genuine danger: data loss, irreversible actions. Used sparingly. |

### Dark theme — "Eclipse"

| Role              | Name        | Hex        | Notes |
|-------------------|-------------|------------|-------|
| Page background   | Eclipse     | `#1A1D24`  | Not pure black. A warm-cool dark. |
| Surface           | Twilight    | `#22262F`  | Toolbar, cards. |
| Surface raised    | Dusk        | `#2B2F39`  | Hover/active surfaces. |
| Border            | Cindery     | `#3A3F4B`  | Borders, dividers. |
| Primary text      | Frost       | `#E4E2DC`  | Body text. Slightly warm off-white. |
| Secondary text    | Mistlight   | `#9A9CA0`  | Captions. |
| Disabled text     | Ashen       | `#6B6D70`  | Disabled. |
| Accent            | Sage        | `#8FB389`  | Brightened sage for dark. |
| Accent deep       | Pine        | `#7BA075`  | Pressed. |
| Link / active     | Tide        | `#7FA4C7`  | Brightened for contrast. |
| Caution           | Ember       | `#D9A276`  | |
| Danger            | Coral       | `#D17863`  | |

### Color rules

1. **Sage is the only brand color.** It is used for focus rings, active indicators, the mark, and primary action buttons. It is *not* used for text on Vellum — for that, use Haze.
2. **Tide is for links and the active tab underline.** It is deliberately *not* the brand color, so links never feel like branding.
3. **No red unless something is genuinely dangerous.** "Delete" gets Coral. A non-fatal warning gets Ember. A download finishing gets nothing.
4. **No gradients.** Halcyon surfaces are flat color. The only gradient permitted is a very subtle vertical Mist→Vellum transition on hero areas of WebUI pages, and it must not exceed 4% lightness change.
5. **No drop shadows on UI chrome.** Surfaces separate by color, not by shadow. Cards on a page may use a single, very soft shadow (`0 1px 3px rgba(0,0,0,0.06)`); the toolbar, tab strip, and menus never do.
6. **Dark mode is not inverted light mode.** Eclipse is its own palette with its own personality — slightly cooler, slightly more contemplative.

## 8. Typography

- **UI font:** Inter, weight 400 (body), 500 (emphasis), 600 (headings). Inter is humanist, modern, reads cleanly at small sizes, and has excellent screen rendering. It is the default Chromium UI font on Linux but Halcyon standardizes on it across all platforms via bundled font fallback.
- **Display / heading:** Inter Tight, weight 500–600. Slightly more compact than Inter, used for the wordmark, NTP hero text, settings section headers, the dialog title bar.
- **Serif (used sparingly for "voice" moments"):** Source Serif 4. Used for:
  - The NTP quote and attribution.
  - The Cove (private mode) greeting sentence.
  - The "About Halcyon" hero paragraph.
  - Margin notes (the user's own writing should feel like writing, not UI).
- **Monospace:** JetBrains Mono. Used for the omnibox in "view source" contexts and for the developer-tools console only. Not used in the regular UI.

### Typographic scale

| Token       | Size / line-height  | Use |
|-------------|---------------------|-----|
| `caption`   | 11px / 14px         | Tab labels (truncated), metadata. |
| `body`      | 13px / 18px         | Default UI body. |
| `body-lg`   | 15px / 22px         | WebUI page body, settings descriptions. |
| `subtitle`  | 14px / 20px, 500    | Setting row titles, dialog sections. |
| `title`     | 17px / 22px, 600    | Section headers, dialog titles. |
| `headline`  | 22px / 28px, 600    | WebUI page h1, NTP secondary text. |
| `display`   | 34px / 40px, 500    | NTP greeting, About hero. |
| `serif-lg`  | 22px / 32px, italic  | NTP quote. |

Chromium normally uses 12px as the UI body size. Halcyon raises the default to 13px and tightens the line-height. The body becomes slightly more readable; the chrome gets slightly more air. This is a deliberate, opinionated departure from upstream.

## 9. Voice & copy

Halcyon's voice is **plain, warm, and slightly dry.** It is never chirpy, never scolding, never promotional. It does not use exclamation marks. It does not greet the user with "Welcome to Halcyon!" on every page. It does not say "Oops!" or "Whoops!" or "Hmm."

It explains, briefly, what is happening, and then gets out of the way.

Examples:

- Chrome: *"Welcome back! Pick up where you left off."*
- Halcyon: *(no welcome back message; tabs simply re-open.)*

- Chrome: *"Oops! Halcyon crashed. Reload?"*
- Halcyon: *"This tab crashed. Reload."*

- Chrome: *"Your connection is not private. Attackers might be trying to steal your information."*
- Halcyon: *"This site's identity can't be verified. Don't enter passwords or payment details here."*

- Chrome: *"Hey there! Halcyon is your browser. Customize it in Settings."*
- Halcyon: *(no onboarding tour. Settings is where Settings has always been: in the menu.)*

Voice rules:

1. **No exclamation marks.** Ever. Not in chrome, not in WebUI, not in error pages. The browser is not excited.
2. **No "Welcome" or "Hi" or "Hello" greetings** outside the NTP greeting (which uses the user's name if known, or "Good morning / afternoon / evening" otherwise, and never says "Welcome to Halcyon").
3. **No marketing copy.** Halcyon does not describe its own features in adjectives. The omnibox placeholder does not say "Search the web faster with Halcyon"; it says "Search or type a URL".
4. **Short sentences.** If a sentence has a comma in the middle, see if it can be two sentences instead.
5. **Plain verbs.** "Save," "Download," "Open," "Close." Not "Save it for later," "Grab this file," "Get going."
6. **Names, not articles, for the product.** The browser refers to itself as "Halcyon," not "the Halcyon browser" and never "your Halcyon."

## 10. Motion

Halcyon motion is **slow on the way in, instant on the way out.**

- Hover states, focus rings, tab transitions: 140ms `cubic-bezier(0.2, 0, 0, 1)`.
- Menus opening: 100ms fade-in. Menus closing: 60ms.
- Tab drag release: 180ms settle.
- NTP greeting fade-in: 600ms. (It is allowed to be slow; it is the first thing the user sees.)
- The Lantern indicator pulse: 1.6s, infinite, but only when the Lantern has something to say. Never decorative.

Anything that the user *initiates* (click, drag, type, scroll) is responsive instantly. Anything the browser initiates (notification, animation, greeting) is allowed to take its time.

There is **no spring physics** in the chrome. Springs are playful; Halcyon is composed.

## 11. Iconography

Halcyon uses a **custom icon set drawn to match the visual language**: 1.5px stroke, rounded line caps, 16px and 20px sizes, no fills (only strokes), built on a 24×24 grid.

Icons are deliberately less "friendly" than Chrome's Material icons — they are more diagrammatic, more like the marks on a map. They use the secondary text color by default and the accent color when active.

See `halcyon/branding/iconography.md` for the full icon catalog.

## 12. What Halcyon does not do

A short, non-exhaustive list of things the upstream browser does that Halcyon deliberately does not:

- **No promotional NTP tiles.** No "shortcuts grid" with sponsored content. No "Discover" feed. The NTP is the Plume (see §13).
- **No sign-in banner.** Halcyon does not prompt to sign in to a Halcyon account on every launch.
- **No "What's New" tour.** Halcyon does not interrupt the user to show new features. The release notes are a single page in Settings, opt-in.
- **No default notification permission for any site.** Sites can request notifications; the user is asked once, quietly, and the default is "Block" with a one-click "Allow for this site."
- **No autoplay audio.** Muted by default across all sites. The user can unmute per-tab.
- **No address-bar "experiments" or "tips."** The omnibox is for typing URLs and searches.
- **No "Continue where you left off" prompt.** Session restore just happens.
- **No shopping annotations in the address bar.** No "price tracking" or "reviews" in the chrome.
- **No Gemini / AI sidebar in the chrome by default.** AI is an opt-in feature, configured in Settings, off out of the box. (See §13 §11.)
- **No GPU process crash "sentinel" banner** that stays until dismissed. A crash is logged; the user is told once, quietly.

## 13. The features (invented for Halcyon)

Each of these is a deliberate extension of the brand promise — "calm the web" — into a specific part of the browsing experience. None are gimmicks.

1. **The Plume** — the new tab page. Not a grid. A single, calm page: a greeting, a quietly rotating quote about attention or stillness, a single search field, and — only on hover — a small list of recently visited. Plus a "today" line where the user can write a one-line intention for their browsing session (kept locally, never synced, never shown to anyone).

2. **Tidemarks** — an optional vertical tab rail on the left edge of the window. Tabs are small cards (favicon + truncated title). Inactive tabs visually recede (lower opacity, smaller). Click to switch. Drag to reorder. Hover the rail to widen it. The rail is off by default; the user turns it on in Settings or with `Cmd/Ctrl + Shift + L`.

3. **Quiet Mode** — a global toggle that suppresses notifications, autoplay, popups, and permission prompts across all tabs. The Lantern indicator turns fully Sage when Quiet Mode is on. Toggle with `Cmd/Ctrl + Shift + M`. Defaults to *on* for new users.

4. **The Lantern** — a small indicator in the toolbar that glows softly when the current site is doing something the user might want to know about (third-party cookies, persistent storage, permission requests, fingerprinting APIs). Click to see a plain-language list. The Lantern never *alerts* — it only *informs on demand*.

5. **Margin Notes** — right-click any page → "Add a margin note." A small card opens in the right margin where the user can write a note about the page. Notes are per-URL, stored locally, optionally sync-able. Notes surface as a tiny mark in the omnibox when the user returns to the page.

6. **Tides** — the history page, reimagined as a river. Tides shows browsing history grouped by day, with each site's "weight" proportional to time spent. The user can "wash away" a site (delete all history for it) by clicking a small tide-pool icon. Tides is a *visualization*, not a list.

7. **Cairn** — the bookmarks system, reimagined as stacks of stones. The default bookmark action adds to the user's current "cairn" (a named stack) rather than to a flat list. The user can have multiple cairns. The cairn icon appears in the toolbar; click to see your stacks and jump to anything in them.

8. **The Cove** — private browsing, reimagined. Calm, dark, with a single sentence explaining what is private and what isn't. No "incognito man in a hat" iconography. No scary spy disclaimer. The Cove is just the part of the beach where the tide doesn't reach.

9. **Anchor tabs** — a tab can be "anchored" (different from bookmarking). Anchored tabs stay open, are visually distinct, group together at the start of the tab strip, and persist across sessions. Useful for web apps you always keep open (mail, calendar, music). Right-click a tab → "Anchor".

10. **Quill** — a built-in, minimal note-taking surface available on any page via `Cmd/Ctrl + Shift + N`. Notes are saved per-origin and live in a side panel. Not for serious writing — for "jot down the thing I just read."

11. **Halcyon Search** (optional) — Halcyon ships with multiple search engines pre-installed and a clean default-picker. The default is DuckDuckGo. The user can choose Google, Brave Search, Startpage, Mojeek, or Halcyon Search (a privacy-respecting meta-search run by the project) on first launch, and change it any time.

12. **Lantern Reports** — a weekly, on-demand, *local-only* summary of what the browser blocked or observed this week. No "your data is sent to Halcyon." It's a single HTML page generated locally and shown in a tab.

## 14. The browser menu

The Halcyon menu is **a single column of plain text labels, with section dividers, and no icons**. It opens from a single button at the top right of the window — the "≡" mark, but rendered as a single horizontal line (the brand mark) rather than three horizontal lines.

Menu structure:

```
   ──────────────────
   New tab                ⌘T
   New window             ⌘N
   New Cove window        ⇧⌘N
   ──────────────────
   Tidemarks              ⇧⌘L
   Quiet Mode          ✓  ⇧⌘M
   ──────────────────
   History
   Bookmarks (Cairn)
   Downloads
   ──────────────────
   Margin notes for this page
   Add to a cairn
   ──────────────────
   Zoom            100%  ⌘+  ⌘-
   Print                  ⌘P
   Find…          ⌘F
   ──────────────────
   Settings                ⌘,
   About Halcyon
   ──────────────────
   Quit Halcyon           ⌘Q
```

No "Cast…", no "Save page as…", no "Send to your devices", no "Create QR code for this page", no "Translate", no "Help" submenu — Help is a single link in Settings. (Translate, share, and other less-frequent actions live in the right-click context menu, where the user can find them when needed.)

## 15. Tone of the whole

Halcyon should feel, to the person using it, like a tool made by someone who has used browsers for a long time and got tired of being interrupted by their own software. It is not a hobby project; it is not a toy; it is a tool. It should feel like the kind of object — a good fountain pen, a well-made kitchen knife, a book bound properly — that the maker took seriously on the user's behalf.

The user does not need to know any of this. They should simply open the browser and find that the web feels quieter than it did.

---

*End of brand book. For implementation details, see the rest of `src/halcyon/branding/` and `src/halcyon/design/`.*
