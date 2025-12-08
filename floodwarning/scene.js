// --- God-Tier Scene Setup ---
const scene = new THREE.Scene();
const camera = new THREE.PerspectiveCamera(75, window.innerWidth / window.innerHeight, 0.1, 1000);
camera.position.set(0, 1, 6); // Pulled back slightly

const renderer = new THREE.WebGLRenderer({
    canvas: document.querySelector('#bg'),
    alpha: true,
    antialias: true, // Smoother edges
});
renderer.setSize(window.innerWidth, window.innerHeight);
renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2)); // Respect device pixel ratio

// --- Mouse Tracking ---
const mouse = new THREE.Vector2();
window.addEventListener('mousemove', (event) => {
    // Normalize mouse position from -1 to 1
    mouse.x = (event.clientX / window.innerWidth) * 2 - 1;
    mouse.y = -(event.clientY / window.innerHeight) * 2 + 1;
});

// --- Lighting ---
const directionalLight = new THREE.DirectionalLight(0xffffff, 0.8);
directionalLight.position.set(5, 10, 7.5);
scene.add(directionalLight);

const ambientLight = new THREE.AmbientLight(0xffffff, 1.2);
scene.add(ambientLight);

// Mouse-tracking point light for highlights
const pointLight = new THREE.PointLight(0x0099ff, 3, 10, 2);
pointLight.position.set(0, 2, 3);
scene.add(pointLight);

const clock = new THREE.Clock();

// --- 3D Objects ---
const masterGroup = new THREE.Group();
scene.add(masterGroup);

// 1. Water Plane
const waterGeometry = new THREE.PlaneGeometry(30, 30, 70, 70); // More vertices
const waterMaterial = new THREE.MeshStandardMaterial({
    color: 0x007bff,
    transparent: true,
    opacity: 0.75,
    metalness: 0.4,
    roughness: 0.2,
});
const water = new THREE.Mesh(waterGeometry, waterMaterial);
water.rotation.x = -Math.PI / 2;
water.position.y = -2;
masterGroup.add(water);

// 2. Sensor Housing
const sensorGroup = new THREE.Group();

const bodyMaterial = new THREE.MeshStandardMaterial({ 
    color: 0x888888,
    metalness: 0.8,
    roughness: 0.4,
});
const bodyGeometry = new THREE.CylinderGeometry(0.4, 0.4, 2.5, 32);
const sensorBody = new THREE.Mesh(bodyGeometry, bodyMaterial);
sensorGroup.add(sensorBody);

const lightGeometry = new THREE.SphereGeometry(0.2, 32, 32);
const lightMaterial = new THREE.MeshStandardMaterial({
    color: 0x00ff00,
    emissive: 0x00ff00,
    emissiveIntensity: 1,
});
const indicatorLight = new THREE.Mesh(lightGeometry, lightMaterial);
indicatorLight.position.y = 1.5;
sensorGroup.add(indicatorLight);

masterGroup.add(sensorGroup);

// --- Scroll-Driven Animation ---
function setupScrollAnimation() {
    const tl = gsap.timeline({
        scrollTrigger: {
            trigger: "body",
            start: "top top",
            end: "bottom bottom",
            scrub: 1,
        }
    });

    tl.to(sensorGroup.rotation, { y: Math.PI * 1.5 }, 0);
    tl.to(camera.position, { z: 8, y: 0 }, 0);
    tl.to(water.position, { y: 0.5 }, 0);
    tl.to(indicatorLight.material.color, { r: 1, g: 0, b: 0 }, 0.5);
    tl.to(indicatorLight.material, { emissive: new THREE.Color(0xff0000) }, 0.5);
}
setupScrollAnimation();

// --- Animation Loop ---
function animate() {
    const elapsedTime = clock.getElapsedTime();

    // Animate water
    const positions = water.geometry.attributes.position.array;
    for (let i = 0; i < positions.length; i += 3) {
        const x = positions[i];
        const y = positions[i + 1];
        positions[i + 2] = Math.sin(x * 0.3 + elapsedTime * 0.5) * 0.15 + Math.sin(y * 0.4 + elapsedTime * 0.8) * 0.1;
    }
    water.geometry.attributes.position.needsUpdate = true;

    // Update mouse-tracking light
    pointLight.position.x = mouse.x * 5;
    pointLight.position.y = mouse.y * 2 + 1;

    // Update scene rotation based on mouse (parallax)
    // Use GSAP for smooth interpolation
    gsap.to(masterGroup.rotation, {
        y: mouse.x * 0.3,
        x: -mouse.y * 0.2,
        duration: 2,
        ease: "power2.out"
    });

    renderer.render(scene, camera);
    requestAnimationFrame(animate);
}
animate();

// --- Handle Window Resize ---
window.addEventListener('resize', () => {
    camera.aspect = window.innerWidth / window.innerHeight;
    camera.updateProjectionMatrix();
    renderer.setSize(window.innerWidth, window.innerHeight);
    renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
});