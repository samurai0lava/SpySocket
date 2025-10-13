// Dark Mode Implementation with Cookies Persistence

// Cookie utility functions
function setCookie(name, value, days) {
    let expires = "";
    if (days) {
        const date = new Date();
        date.setTime(date.getTime() + (days * 24 * 60 * 60 * 1000));
        expires = "; expires=" + date.toUTCString();
    }
    document.cookie = name + "=" + (value || "") + expires + "; path=/";
}

function getCookie(name) {
    const nameEQ = name + "=";
    const ca = document.cookie.split(';');
    for (let i = 0; i < ca.length; i++) {
        let c = ca[i];
        while (c.charAt(0) == ' ') c = c.substring(1, c.length);
        if (c.indexOf(nameEQ) == 0) return c.substring(nameEQ.length, c.length);
    }
    return null;
}

function eraseCookie(name) {
    document.cookie = name + '=; Path=/; Expires=Thu, 01 Jan 1970 00:00:01 GMT;';
}

// Function to apply dark mode
function applyDarkMode(isDark) {
    if (isDark) {
        document.body.classList.add('dark-mode');
    } else {
        document.body.classList.remove('dark-mode');
    }
}

// Function to get dark mode preference from cookie
function getDarkModePreference() {
    const preference = getCookie('darkMode');
    return preference === 'true';
}

// Function to set dark mode preference in cookie (expires in 365 days)
function setDarkModePreference(isDark) {
    setCookie('darkMode', isDark.toString(), 365);
}

// Function to toggle dark mode
function toggleDarkMode() {
    const isDark = !document.body.classList.contains('dark-mode');
    applyDarkMode(isDark);
    setDarkModePreference(isDark);
}

// Initialize dark mode on page load
document.addEventListener('DOMContentLoaded', function() {
    // Apply saved preference from cookie
    const isDark = getDarkModePreference();
    applyDarkMode(isDark);
    
    // Add event listener to toggle button
    const toggleButton = document.getElementById('darkModeToggle');
    if (toggleButton) {
        toggleButton.addEventListener('click', toggleDarkMode);
    }
});

// Legacy function for backward compatibility
function darkmode() {
    toggleDarkMode();
}