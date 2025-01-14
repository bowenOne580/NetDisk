const button = document.querySelector(".bt");
const input = document.querySelector(".inp");
const content = document.querySelector(".PrimeHere");

button.addEventListener("click",GetPrime);

function GetPrime(){
    const resp = fetch(`/get/prime?n=${input.value}`);
    resp.then((response) =>{
        if (response.ok){
            return response.json();
        }
        throw Error("Get Wrong Response");
    })
    .then((data)=>{
        console.log(data.prime);
        if (data.prime!==-1) content.textContent = "The "+input.value+" th prime is "+data.prime+"\n";
        else content.textContent = "Number too large!\n";
    })
}