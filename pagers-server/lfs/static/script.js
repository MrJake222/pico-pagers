p = document.getElementById("pager-list")

for (let i=0; i<20; i++) {
    let ch = document.createElement("div")
    ch.innerHTML = "pager " + (i+1)
    p.appendChild(ch)
}