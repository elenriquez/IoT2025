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
    const navbar = document.querySelector(".navbar");
    const navLinks = document.querySelectorAll('.desktop-nav a');
    const navLinksArray = Array.from(navLinks);
    const desktopNav = document.querySelector('.desktop-nav');

    let previousActiveIndex = -1;

    function setActiveLink(targetLink) {
        const newActiveIndex = navLinksArray.findIndex(link => link === targetLink);

        if (newActiveIndex === -1) return;

        if (previousActiveIndex !== -1 && newActiveIndex !== previousActiveIndex) {
            const direction = newActiveIndex > previousActiveIndex ? 'right' : 'left';
            if (direction === 'right') {
                desktopNav.classList.add('nav-to-right');
                desktopNav.classList.remove('nav-to-left');
            } else {
                desktopNav.classList.add('nav-to-left');
                desktopNav.classList.remove('nav-to-right');
            }
        }
        
        navLinks.forEach(l => l.classList.remove('active'));
        targetLink.classList.add('active');
        previousActiveIndex = newActiveIndex;
    }

    navLinks.forEach(link => {
        link.addEventListener('click', (e) => {
            if (link.getAttribute('href').startsWith('#')) {
                setActiveLink(link);
            }
        });
    });

    // Active Link Highlighting
    const sections = document.querySelectorAll("section[id]");
    sections.forEach(section => {
        ScrollTrigger.create({
            trigger: section,
            start: "top center+=100",
            end: "bottom center-=100",
            onToggle: self => {
                if (self.isActive) {
                    const targetLink = document.querySelector(`.desktop-nav a[href="#${section.id}"]`);
                    if (targetLink) {
                        setActiveLink(targetLink);
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
