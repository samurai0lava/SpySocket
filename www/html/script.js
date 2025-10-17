// Cookie utilities
function setCookie(name, value, days) {
  let expires = "";
  if (days) {
    const date = new Date();
    date.setTime(date.getTime() + days * 24 * 60 * 60 * 1000);
    expires = "; expires=" + date.toUTCString();
  }
  document.cookie = `${name}=${value || ""}${expires}; path=/`;
}

function getCookie(name) {
  const nameEQ = name + "=";
  const ca = document.cookie.split(";");
  for (let c of ca) {
    while (c.charAt(0) === " ") c = c.substring(1, c.length);
    if (c.indexOf(nameEQ) === 0) return c.substring(nameEQ.length, c.length);
  }
  return null;
}

function setDarkModePreference(isDark) {
  setCookie("darkMode", isDark.toString(), 365);
}

function getDarkModePreference() {
  return getCookie("darkMode") === "true";
}

document.addEventListener("DOMContentLoaded", function () {
  const body = document.body;
  const toggle = document.getElementById("darkModeToggle");

  const mainServer = document.querySelector(".server-illustration");
  const window1 = document.querySelector(".server-window1");
  const window2 = document.querySelector(".server-window2");
  const logo1337 = document.querySelector(".logo-1337");
  const exploreIcon = document.querySelector(".exp-btn");
  const githubIcon = document.querySelector(".git-btn");
  const teamIcon = document.querySelector(".team-icon");
  const homeIcon = document.querySelector(".home-btn-image");

  function enableDarkMode() {
    body.classList.add("dark-mode");
    if (mainServer) mainServer.src = "img/light-server.png";
    if (window1) window1.src = "img/light-image8.png";
    if (window2) window2.src = "img/image-10-light.png";
    if (logo1337) logo1337.src = "img/light-1337.png";
    if (exploreIcon) exploreIcon.src = "img/light-application.png";
    if (githubIcon) githubIcon.src = "img/light-github.png";
    if (teamIcon) teamIcon.src = "img/light-group-chat.png";
    if (homeIcon) homeIcon.src = "img/light-home.png";
    setDarkModePreference(true);
  }

  function disableDarkMode() {
    body.classList.remove("dark-mode");
    if (mainServer) mainServer.src = "img/Server-vector.png";
    if (window1) window1.src = "img/image-8-1 .png";
    if (window2) window2.src = "img/image-8.png";
    if (logo1337) logo1337.src = "img/1337.png";
    if (exploreIcon) exploreIcon.src = "img/application.png";
    if (githubIcon) githubIcon.src = "img/Github.png";
    if (teamIcon) teamIcon.src = "img/group-chat.png";
    if (homeIcon) homeIcon.src = "img/home.png";
    setDarkModePreference(false);
  }

  // Load saved preference
  if (getDarkModePreference()) {
    enableDarkMode();
  }

  // Toggle on button click
  if (toggle) {
    toggle.addEventListener("click", () => {
      if (body.classList.contains("dark-mode")) {
        disableDarkMode();
      } else {
        enableDarkMode();
      }
    });
  }
});
