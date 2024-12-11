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
      document.getElementById('status_firmware').innerText = value_json.status.firmware;
      document.getElementById('status_serial').innerText = value_json.status.serial;
      document.getElementById('status_siInitialised').innerText = value_json.status.siInitialised;
      document.getElementById('status_mqtt').innerText = value_json.status.mqtt;
      document.getElementById('espa_model').innerText = value_json.eSpa.model;
      document.getElementById('espa_build').innerText = value_json.eSpa.update.installed_version;
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

// Retrieving and updating the configured settings, so they can be displayed in the modal popup
function loadConfig() {
    $('#configErrorAlert').hide();
    fetch('/json/config')
        .then(response => response.json())
        .then(data => {
            document.getElementById('spaName').value = data.spaName;
            document.getElementById('mqttServer').value = data.mqttServer;
            document.getElementById('mqttPort').value = data.mqttPort;
            document.getElementById('mqttUsername').value = data.mqttUsername;
            document.getElementById('mqttPassword').value = data.mqttPassword;
            document.getElementById('updateFrequency').value = data.updateFrequency;
        })
        .catch(error => {
            console.error('Error loading config:', error);
            $('#configErrorAlert').show(); // Show the alert if there's an error
    });
}

// Modal dialog content population
$(document).ready(function() {
    // JSON dump modal
    $('#jsonLink').click(function(event) {
        event.preventDefault();
        fetch('/json').then(response => response.json()).then(data => {
            $('#infoModalTitle').html("Spa JSON");
            $('#infoModalBody').html('<pre>' + JSON.stringify(data, null, 2) + '</pre>');
            $('#infoModal').modal('show');
        }).catch(error => console.error('Error fetching JSON:', error));
    });

    // spa status modal
    $('#statusLink').click(function(event) {
        event.preventDefault();
        fetch('/status').then(response => response.json()).then(data => {
            $('#infoModalTitle').html("Spa Status");
            $('#infoModalBody').html('<pre>' + data + '</pre>');
            $('#infoModal').modal('show');
        }).catch(error => console.error('Error fetching status:', error));
    });

    // configuration settings modal
    $('#configLink').click(function(event) {
        event.preventDefault();
        $('#configModal').modal('show');
    });
    // Load configuration when the config modal is shown
    $('#configModal').on('shown.bs.modal', function() {
        loadConfig();
    });
    // Handle form submission when the save button is clicked
    $('#saveConfigButton').click(function() {
        submitConfigForm();
    });
    function submitConfigForm() {
        $.ajax({
            url: '/config',
            type: 'POST',
            data: $('#config_form').serialize(),
            success: function() {
                // $('#msg').html('<p style="color:green;">Configuration updated successfully!</p>');
                loadConfig();
                setTimeout(function() { $('#msg').html(''); }, 3000);
                $('#configModal').modal('hide');
            },
            error: function() {
                $('#configErrorAlert').text('Error updating configuration. Please try again.').show();
            }
        });
    }

    $('#config_form').submit(function(e) {
        e.preventDefault();
        submitConfigForm();
    });
});
