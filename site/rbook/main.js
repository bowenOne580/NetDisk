const button1 = document.querySelector(".colb");
const button2 = document.querySelector(".prob");
const button3 = document.querySelector(".remove");
const path = document.querySelector(".path");

button1.addEventListener("click",f1);
button2.addEventListener("click",f2);
button3.addEventListener("click",f3);

form1.action = `/rbook/upload?expath=${path.textContent}`;
form2.action = `/rbook/delete?expath=${path.textContent}`;

function f1(){
    const userInput = prompt("Enter New Column Name");
    if (userInput === null) return;
    if (userInput === ""){
        alert("Name cannot be empty!");
        return;
    }
    if (path.textContent[0] === '!'){
        alert("Invalid Path");
        return;
    }
    let s = path.textContent+"/"+userInput;
    // console.log(s);
    const resp = fetch(`/get/rbook?expath=${s}`);
    // resp.then(document.location.assign(`/rbook?expath=${path.textContent}`));
    resp.then(document.location.assign(`/rbook?expath=${path.textContent}`));
}

function f2(){
    let s = path.textContent;
    document.location.assign(`/rbook/addproblem?expath=${s}`);
}

function f3(){
    const userInput = prompt("Enter Directory to Delete");
    if (userInput === null) return;
    if (userInput === ""){
        alert("Name cannot be empty!");
        return;
    }
    if (path.textContent[0] === '!'){
        alert("Invalid Path");
        return;
    }
    let s = path.textContent+"/"+userInput;
    const resp = fetch(`/remove/rbook?expath=${s}`);
    resp.then(document.location.reload());
}

function validate(){
    let fileIn = document.getElementById("fileInput");
    let file = fileIn.files;
    console.log(file.length);
    if (file.length === 0){
        alert("No file is selected!");
        return false;
    }
    else return true;
}
