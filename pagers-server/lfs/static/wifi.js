function setScanningStatus(msg) {
    document.getElementById("scanningStatus").innerHTML = msg;
}

function startScan() {
    setScanningStatus('Making a request...');
    fetch(URL_PREFIX + "/wifi/scan/start")
        .then(data => {
            console.log(data);
            setTimeout(checkScanStatus, 500);
            setScanningStatus('Scanning in progress...');
        })
        .catch(error => {
            console.error("Error starting scan:", error);
            setScanningStatus('Failed to connect to the server!');
        });
}

function checkScanStatus() {
    fetch(URL_PREFIX + "/wifi/scan/status")
        .then(response => response.json())
        .then(data => {
            console.log(data);
            if (data.active) {
                setTimeout(checkScanStatus, 1000);
            } else {
                console.log('finished!')
                setScanningStatus('Finished scanning! Retrieving list of SSIDs...');
                showScanResults();
            }
        })
        .catch(error => {
            console.error("Error checking scan status:", error);
            setScanningStatus('Failed to connect to the server!');
        });
}

function showScanResults() {
    const wifiList = document.getElementById("wifiList");
    wifiList.innerHTML = "<li>Scanning...</li>"; // Display "Scanning..." text

    fetch(URL_PREFIX + "/wifi/scan/results")
        .then(response => response.json())
        .then(data => {
            console.log(data);
            wifiList.innerHTML = ""; // Clear previous "Scanning..." text

            Object.values(data).forEach(network => {
                const listItem = document.createElement("li");
                listItem.textContent = network.ssid;
                listItem.addEventListener("click", () => {
                    document.getElementById("ssidInput").value = network.ssid;
                });
                wifiList.appendChild(listItem);
            });
        })
        .catch(error => {
            console.error("Error retrieving scan results:", error);
        });
}

function connectToWiFi(ssid) {
    const ssidInput = document.getElementById("ssidInput");
    const passwordInput = document.getElementById("passwordInput");

    const selectedSSID = ssid || ssidInput.value;
    const password = passwordInput.value;

    fetch(URL_PREFIX + `/wifi/connect?ssid=${selectedSSID}&password=${password}&auth=4`)
        .then(data => {
            console.log(data);
            checkConnectStatus();
        })
        .catch(error => {
            console.error("Error connecting to WiFi:", error);
        });
}

function checkConnectStatus() {
    fetch(URL_PREFIX + "/wifi/connect/status")
        .then(response => response.json())
        .then(data => {
            console.log(data);
            if (data.success) {
                alert('Connected to the wifi!')
            } else {
                setScanningStatus('Failed to connect to the wifi... Trying again...');
                setTimeout(checkConnectStatus, 1000);
            }
        })
        .catch(error => {
            console.error("Error checking connection status:", error);
        });
}