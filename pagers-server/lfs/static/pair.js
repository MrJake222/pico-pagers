function pairPager() {
    console.log('Pairing...');
    const id = 999;
    fetch(URL_PREFIX + `/pagers/pair?id=${id}`)
        .then(data => {
            console.log(data);
        })
        .catch(error => {
            console.error("Error pairing init:", error);
        });
}