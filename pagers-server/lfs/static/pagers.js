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

                const flashButton = document.createElement("button");
                flashButton.innerHTML = 'flash';
                flashButton.onclick = function () {
                  flashPager(pagerName, 30);
                  console.log('flashing', pagerName);
                };

                const abortFlashButton = document.createElement("button");
                abortFlashButton.innerHTML = 'abort';
                abortFlashButton.onclick = function () {
                  flashPager(pagerName, 0);
                  console.log('aborting', pagerName);
                };

                const removeButton = document.createElement("button");
                removeButton.innerHTML = 'remove';
                removeButton.onclick = function () {
                  removePager(pagerName);
                  console.log('removing', pagerName);
                };

                const repairButton = document.createElement("button");
                repairButton.innerHTML = 're-pair';
                repairButton.onclick = function () {
                  pairPagerId(pagerName);
                  console.log('re-pairing', pagerName);
                };

                spanNameElement.textContent = pagerName;// + ": ";
                spanValueElement.textContent = "";//pagerValue;
                liElement.appendChild(spanNameElement);
                liElement.appendChild(spanValueElement);
                liElement.appendChild(flashButton);
                liElement.appendChild(abortFlashButton);
                liElement.appendChild(removeButton);
                liElement.appendChild(repairButton);
                ulElement.appendChild(liElement);
            }
        }

    })
    .catch(error => {
        console.error("Error pairing init:", error);
    });
}

function flashPager(id, time) {
    fetch(URL_PREFIX + `/pagers/flash?id=${id}&time=${time}`)
        .then(data => {
            console.log(data);
            // loadPagers();
        })
        .catch(error => {
            console.error("Error flashing:", error);
        });
}

function removePager(id) {
    fetch(URL_PREFIX + `/pagers/remove?id=${id}`)
        .then(data => {
            console.log(data);
            loadPagers();
        })
        .catch(error => {
            console.error("Error removing:", error);
        });
}