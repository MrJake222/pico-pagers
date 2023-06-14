function loadPagers() {
    const ulElement = document.getElementById("pagerList");
    fetch(URL_PREFIX + `/pagers/list`)
        .then(data => data.json())
    .then(data => {
        console.log(data);
        ulElement.innerHTML = '';
        for (const key in data.pagers) {
            if (data.pagers.hasOwnProperty(key)) {
                const pagerName = key;
                const pagerValue = data.pagers[key];

                const liElement = document.createElement("li");
                const spanNameElement = document.createElement("span");
                const spanValueElement = document.createElement("span");

                const button = document.createElement("button");
                button.innerHTML = 'flash';
                button.onclick = function () {
                  flashPager(pagerName);
                  console.log('flashing', pagerName);
                };

                spanNameElement.textContent = pagerName + ": ";
                spanValueElement.textContent = pagerValue;
                liElement.appendChild(spanNameElement);
                liElement.appendChild(spanValueElement);
                liElement.appendChild(button);
                ulElement.appendChild(liElement);
            }
        }

    })
    .catch(error => {
        console.error("Error pairing init:", error);
    });
}

function flashPager(id) {
    fetch(URL_PREFIX + `/pagers/flash?id=${id}`)
        .then(data => {
            console.log(data);
            loadPagers();
        })
        .catch(error => {
            console.error("Error pairing init:", error);
        });
}