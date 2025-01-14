const button = document.querySelector(".sub");
const Name = document.querySelector(".inName");
const desc = document.querySelector(".inDes");
const sol = document.querySelector(".inSol");
const s = document.querySelector(".path");

button.addEventListener("click",f);

function f(){
    let path = s.textContent+"/"+Name.value;
    resp = fetch(`/get/rbook/prob?expath=${path}&desc=${desc.value}&sol=${sol.value}`);
    document.location.assign(`/rbook?expath=${path}`);
}