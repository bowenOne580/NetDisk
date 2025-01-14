const button = document.querySelector(".vis");
const sol = document.querySelector(".sol");
let s = sol.textContent;
sol.textContent = "";

button.addEventListener("click",f);

function f(){
    if (button.textContent === "Show"){
        button.textContent = "Hide";
        sol.textContent = s;
    }
    else{
        button.textContent = "Show";
        sol.textContent = "";
    }
}