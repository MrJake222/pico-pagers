function pairPager() {
    console.log('Pairing...');
    const id = document.getElementById("pagerIdInput").value;
    if (!id) {
        alert('Wrong id!');
        return;
    }
    alert(`Pairing ${id}... Click pairing button on your device!`);
    fetch(URL_PREFIX + `/pagers/pair?id=${id}`)
        .then(data => {
            console.log(data);
        })
        .catch(error => {
            console.error("Error pairing init:", error);
        });
}