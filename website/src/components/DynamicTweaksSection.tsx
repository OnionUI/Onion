import React, { useEffect } from 'react';

export default function DynamicTweaksSection({ children }) {
    useEffect(() => {
        const handleHashChange = async () => {
            const target = location.hash && document.querySelector(location.hash);

            if (target) {
                const openParents = [];
                let currentElement = target;

                // Close (collapse) all other open details elements
                const openDetails = document.querySelectorAll('details[open]');
                openDetails.forEach((openDetail) => {
                    if (openDetail !== target.parentElement) {
                        const summary = openDetail.querySelector('summary');
                        if (summary) {
                            summary.click();
                        }
                    }
                });

                // Introduce a slight delay before processing openParents
                await new Promise(resolve => setTimeout(resolve, 300));

                while (currentElement && currentElement.tagName !== 'body') {
                    const details = currentElement.closest('details');
                    if (details && details instanceof HTMLDetailsElement) {
                        openParents.push(details);
                    }
                    currentElement = currentElement.parentElement;
                }

                if (openParents.length > 0) {
                    openParents.reverse(); // Start with the outermost parent
                    openParents.forEach((parent) => {
                        if (!parent.open) {
                            const summary = parent.querySelector('summary');
                            if (summary) {
                                summary.click();
                            }
                        }
                    });
                }

                // Add a slight scrolling effect
                await new Promise(resolve => setTimeout(resolve, 300));  // wait the end of the just opened item
                const offsetPosition = target.getBoundingClientRect().top + window.scrollY - 85;
                window.scrollTo({
                    top: offsetPosition,
                    behavior: 'smooth'
                });
            }
        };

        // Add event listener for hash changes
        window.addEventListener('hashchange', handleHashChange);

        // Initial call to handle the current hash
        handleHashChange();

        // Cleanup the event listener when the component unmounts
        return () => {
            window.removeEventListener('hashchange', handleHashChange);
        };
    }, []);

    return <>{children}</>;
}
