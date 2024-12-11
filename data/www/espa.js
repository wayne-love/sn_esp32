function confirmAction(url) {
    if (confirm('Are you sure?')) {
    window.location.href = url;
    }
}

function fetchStatus() {
  fetch('/json')
    .then(response => response.json())
    .then(value_json => {
      document.getElementById('temperatures_water').innerText = value_json.temperatures.water + "\u00B0C";
      document.getElementById('temperatures_setPoint').value = value_json.temperatures.setPoint;
      document.getElementById('status_state').innerText = value_json.status.state;
      document.getElementById('status_controller').innerText = value_json.status.controller;
      document.getElementById('status_serial').innerText = value_json.status.serial;
      document.getElementById('status_siInitialised').innerText = value_json.status.siInitialised;
      document.getElementById('status_mqtt').innerText = value_json.status.mqtt;
    })
    .catch(error => console.error('Error fetching status:', error));
}

function updateTempSetPoint() {
  const temperatures_setPoint = document.getElementById('temperatures_setPoint').value;
  fetch('/set', {
    method: 'POST',
    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
    body: 'temperatures_setPoint=' + temperatures_setPoint
  })
  .then(response => response.text())
  .then(result => console.log(result))
  .catch(error => console.error('Error setting temperature:', error));
}

function sendCurrentTime() {
  const status_datetime = new Date(Date() + " UTC").toISOString().slice(0, 19).replace("T", " ");
  fetch('/set', {
    method: 'POST',
    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
    body: 'status_datetime=' + status_datetime
  })
  .then(response => response.text())
  .then(result => console.log(result))
  .catch(error => console.error('Error setting temperature:', error));
}

window.onload = function() {
    fetchStatus();
    setInterval(fetchStatus, 10000);
}

// Modal dialog content population
$(document).ready(function() {
    $('#jsonLink').click(function(event) {
        event.preventDefault();
        fetch('/json').then(response => response.json()).then(data => {
            $('#infoModalTitle').html("Spa JSON");
            $('#infoModalBody').html('<pre>' + JSON.stringify(data, null, 2) + '</pre>');
            $('#infoModal').modal('show');
        }).catch(error => console.error('Error fetching JSON:', error));
    });
    $('#statusLink').click(function(event) {
        event.preventDefault();
        fetch('/status').then(response => response.json()).then(data => {
            $('#infoModalTitle').html("Spa Status");
            $('#infoModalBody').html('<pre>' + data + '</pre>');
            $('#infoModal').modal('show');
        }).catch(error => console.error('Error fetching status:', error));
    });
    // $('#configLink').click(function(event) {
    //     event.preventDefault();
    //     fetch('/status').then(response => response.json()).then(data => {
    //         $('#infoModalTitle').html("Spa Status");
    //         $('#infoModalBody').html('<pre>' + data + '</pre>');
    //         $('#infoModal').modal('show');
    //     }).catch(error => console.error('Error fetching status:', error));
    // });
});