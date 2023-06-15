function pairPagerId(id) {
    console.log('Pairing...');
    if (!id) {
        alert('Wrong id!');
        return;
    }
    alert(`Pairing ${id}... Click pairing button on your device!`);
    fetch(URL_PREFIX + `/pagers/pair?id=${id}`)
        .then(data => {
            console.log(data);
            loadPagers();
        })
        .catch(error => {
            console.error("Error pairing init:", error);
        });
}

function pairPager() {
    const id = document.getElementById("pagerIdInput").value;
    pairPagerId(id)
}