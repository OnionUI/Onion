import React, { useEffect } from 'react';

export default function DynamicTweaksSection({ children }) {
    useEffect(() => {
        const target = location.hash && document.querySelector(location.hash);
        
        if (target) {
            const openParents = [];
            let currentElement = target;

            while (currentElement && currentElement.tagName !== 'body') {
                if (currentElement.tagName === 'details') {
                    openParents.push(currentElement);
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
        }
    }, []);

    return <>{children}</>;
}
