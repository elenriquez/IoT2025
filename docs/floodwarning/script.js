document.addEventListener("DOMContentLoaded", function() {
    gsap.registerPlugin(ScrollTrigger);

    // --- Navbar Logic ---
    const hamburger = document.querySelector(".hamburger");
    const mobileNav = document.querySelector(".mobile-nav");
    const body = document.querySelector("body");

    hamburger.addEventListener("click", () => {
        hamburger.classList.toggle("is-active");
        mobileNav.classList.toggle("is-active");
        body.classList.toggle("mobile-nav-open");
    });

    mobileNav.querySelectorAll('a').forEach(link => {
        link.addEventListener('click', () => {
            hamburger.classList.remove("is-active");
            mobileNav.classList.remove("is-active");
            body.classList.remove("mobile-nav-open");
        });
    });

    const navbar = document.querySelector(".navbar");
    const navLinks = document.querySelectorAll('.desktop-nav a');

    // Active Link Highlighting
    const sections = document.querySelectorAll("section[id]");
    sections.forEach(section => {
        ScrollTrigger.create({
            trigger: section,
            start: "top center+=100",
            end: "bottom center-=100",
            onToggle: self => {
                if (self.isActive) {
                    navLinks.forEach(link => link.classList.remove('active'));
                    const targetLink = document.querySelector(`.desktop-nav a[href="#${section.id}"]`);
                    if (targetLink) {
                        targetLink.classList.add('active');
                    }
                }
            }
        });
    });

    // --- God-Tier Animations ---
    // Hero Animation
    gsap.from(".hero-title", { opacity: 0, y: 50, duration: 1.5, ease: "power4.out", delay: 0.5 });
    gsap.from(".hero-tagline", { opacity: 0, y: 30, duration: 1.2, ease: "power4.out", delay: 0.8 });

    // Choreographed Section Reveals
    const contentSections = document.querySelectorAll(".content-section");
    contentSections.forEach((section) => {
        const sectionTitle = section.querySelector(".section-title");
        const cards = section.querySelectorAll(".card");
        const slideshow = section.querySelector(".slideshow-container");

        const sectionTl = gsap.timeline({
            scrollTrigger: {
                trigger: section,
                start: "top 85%",
                toggleActions: "play none none none",
            },
            defaults: { ease: "power4.out", duration: 1 }
        });

        if (sectionTitle) {
            sectionTl.from(sectionTitle, { opacity: 0, y: 50 });
        }
        if (cards.length > 0) {
            sectionTl.from(cards, { opacity: 0, y: 40, stagger: 0.15 }, "-=0.8");
        }
        if (slideshow) {
            sectionTl.from(slideshow, { opacity: 0, y: 50 }, "-=0.8");
        }
    });
});
