# Flood Warning Web Page

This folder contains the Flood Warning project page served from the `docs` directory for GitHub Pages.

## Structure
- `index.html` – main page for the Flood Warning project
- `style.css` – page styling
- `script.js` – UI interactions
- `scene.js` – background/animation setup
- Images are stored in the shared `docs/imgs/` folder with names `flood_doc1.jpg` … `flood_doc6.jpg` and `flood_schema.png`.
- `firmware/` – Arduino sketches: `Float_Sensor.ino`, `Traffic_Receiver.ino`, and `MAC_Address_Finder.ino`.

## Viewing
- GitHub Pages: the site is served from `docs/floodwarning/` via the main `docs/index.html` link.
- Local preview: open `docs/floodwarning/index.html` in a browser. Ensure the `docs/imgs/` folder is present so images load correctly.

## Notes
- Top-level `floodwarning/` (outside `docs`) is kept as a backup; the version under `docs/` is the one used for deployment.
- External libs loaded via CDN: three.js and GSAP/ScrollTrigger.
