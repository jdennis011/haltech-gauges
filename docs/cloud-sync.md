# Cloud sync: the hub and the online account

A hub linked to an account at oagauge.online keeps its library in step with it: configs, images,
theme colours and alerts. Make a face on the site from the sofa and the hub in the car has it
within a minute of the next time it is on the home network; change a colour on the hub's own
page and the site has it straight away. Neither side needs the other to work; the link only
decides what follows what.

## Linking

1. On the site, signed in: **Settings**, **Hubs**, **Link a hub**. A six-character code appears,
   good for ten minutes.
2. On the hub's page: **Settings**, **Online account**, type the code, **Link**. The hub has to be
   on the home network (section "Home Wi-Fi" above it); the access point alone cannot reach the
   site.
3. The card shows the account, the hub's id and name, the state of the link and the last sync.
   The site's Hubs card lists the hub with its board, firmware, when it last checked in and how
   many gauges it had online.

**Unlink** on either side ends it. Nothing is deleted on either side; they just stop following
each other. A hub linked to a second account leaves the first.

## What syncs, and which way

| Item | Hub to site | Site to hub |
|---|---|---|
| Configs (the library) | On save or delete on the hub's page, straight away | Within a minute: new, changed and deleted |
| Images | The same | The same |
| Theme colours | On change | Within a minute |
| Alerts | On change | Within a minute |
| Planned gauges, assignments, recordings | No | No: they belong to the hub and its hardware |

- Each side compares by content (the CRC the hub already shows for configs), so nothing is sent
  twice and an unchanged item costs one small request a minute.
- When both sides changed the same item, the site's copy wins, because the hub sends its own
  changes up as they happen and so is only ever behind the site, never the other way.
- A deletion on the site reaches a hub that has synced since the item existed; a hub that has
  never seen the account sends its own copies up instead, so linking a hub with faces on it
  adds them to the account rather than wiping them.
- A config the hub's own parser refuses (the site checks less strictly) is skipped and named in
  the card's state; everything else still syncs.

## How it works

- The hub calls the site over **HTTPS** with the car's home network, checking the site's
  certificate against the Mozilla root bundle the Arduino core carries, so the token cannot be
  read on the way.
- The code is exchanged once for a **token** that stands in for a sign-in on the account's
  storage endpoints (`Authorization: Bearer`). The hub keeps it in its settings flash, never shows
  it on its page, and the site keeps only a hash of it. Unlinking revokes it.
- Every write to the account bumps a **revision**; the hub asks `GET /api/hubs/sync` once a
  minute, gets the account's manifest (names, CRCs, deletions with the revision they happened at,
  the revision of the theme colours and alerts), and fetches only what differs. The hub remembers
  the revision it last applied, which is how it tells a deletion it should follow from an item it
  should send up.
- The hub checks in with its state every five minutes (`POST /api/hubs/status`) for the site's
  Hubs card.
- It all runs on a task of its own on the hub, one request at a time, and waits when the hub has
  less than 96 KB free, since a TLS connection needs about 45 KB. On the C3 development board
  that is most of the time; the C6 and the P4 have room.

### Endpoints added to the site

| | |
|---|---|
| `POST /api/hubs/code` (signed in) | `{code, expires}`: a link code, ten minutes |
| `POST /api/hubs/claim` `{code, hubId, name, fw, board}` | `{token, email}`; the code is used up |
| `GET /api/hubs` (signed in) | The account's hubs with their last state |
| `DELETE /api/hubs/{id}` (signed in, or the hub itself) | Unlinks |
| `GET /api/hubs/sync` (hub token) | The manifest |
| `POST /api/hubs/status` (hub token) | The hub's check-in |
| The config, image and settings endpoints | Take the hub token as well as the cookie; replies carry the new `rev` |

### Endpoints on the hub

| | |
|---|---|
| `GET /api/cloud` | `{url, hub_id, name, linked, email, state, error, last_sync_s, rev, pending}`; also in `/api/status` as `cloud` |
| `POST /api/cloud/link` `{code}` | Starts linking; the state shows the outcome |
| `POST /api/cloud/unlink`, `POST /api/cloud/sync` | |

The site's address is `HUB_CLOUD_URL` in `src/hub/hub_config.h`.
