
// Replace with your LS_server IP or hostname
const LS_SERVER_BASE_URL = 'http://192.168.4.1'; // Example IP, change as needed

// SVG Bulb Images (Base64)
const BULB_ON = 'data:image/svg+xml;base64,PHN2ZyB3aWR0aD0iNDAiIGhlaWdodD0iNDAiIHZpZXdCb3g9IjAgMCAyNCAyNCIgZmlsbD0ibm9uZSIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIj48ZWxsaXBzZSBjeD0iMTIiIGN5PSI4IiByeD0iNiIgcnk9IjYiIGZpbGw9IiNmZmRiNzAiLz48cmVjdCB4PSIxMCIgeT0iMTQiIHdpZHRoPSI0IiBoZWlnaHQ9IjYiIHJ4PSIyIiBmaWxsPSIjZmZkYjcwIi8+PHJlY3QgeD0iOSIgeT0iMjAiIHdpZHRoPSI2IiBoZWlnaHQ9IjIiIHJ4PSIxIiBmaWxsPSIjY2NjIi8+PC9zdmc+';
const BULB_OFF = 'data:image/svg+xml;base64,PHN2ZyB3aWR0aD0iNDAiIGhlaWdodD0iNDAiIHZpZXdCb3g9IjAgMCAyNCAyNCIgZmlsbD0ibm9uZSIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIj48ZWxsaXBzZSBjeD0iMTIiIGN5PSI4IiByeD0iNiIgcnk9IjYiIGZpbGw9IiNlMGU0ZWEiLz48cmVjdCB4PSIxMCIgeT0iMTQiIHdpZHRoPSI0IiBoZWlnaHQ9IjYiIHJ4PSIyIiBmaWxsPSIjY2NjIi8+PHJlY3QgeD0iOSIgeT0iMjAiIHdpZHRoPSI2IiBoZWlnaHQ9IjIiIHJ4PSIxIiBmaWxsPSIjY2NjIi8+PC9zdmc+';

function updateStatus(light, status) {
	const statusSpan = document.getElementById('status' + light);
	const bulbImg = document.getElementById('bulb' + light);
	const slider = document.getElementById('slider' + light);
	if (status === 'on') {
		statusSpan.textContent = 'ON';
		statusSpan.classList.add('on');
		statusSpan.classList.remove('off');
		bulbImg.src = BULB_ON;
		bulbImg.setAttribute('data-state', 'on');
		if (slider) {
			slider.value = 1;
			slider.classList.add('on');
			slider.classList.remove('off');
		}
	} else {
		statusSpan.textContent = 'OFF';
		statusSpan.classList.add('off');
		statusSpan.classList.remove('on');
		bulbImg.src = BULB_OFF;
		bulbImg.setAttribute('data-state', 'off');
		if (slider) {
			slider.value = 0;
			slider.classList.add('off');
			slider.classList.remove('on');
		}
	}
}

function sliderChange(light) {
	const slider = document.getElementById('slider' + light);
	const newStatus = slider.value === '1' ? 'on' : 'off';
	fetch(`${LS_SERVER_BASE_URL}/relay/${light}/${newStatus}`, { method: 'POST' })
		.then(res => res.json())
		.then(data => {
			updateStatus(light, data.status);
		})
		.catch(() => {
			updateStatus(light, 'OFF');
		});
}

function fetchStatus() {
	for (let i = 1; i <= 3; i++) {
		fetch(`${LS_SERVER_BASE_URL}/status/${i}`)
			.then(res => res.json())
			.then(data => {
				updateStatus(i, data.status);
			})
			.catch(() => {
				updateStatus(i, 'OFF');
			});
	}
}

function setToggleTime(light) {
	const timeInput = document.getElementById(`toggleTime${light}`);
	const timeValue = timeInput.value;
	if (!timeValue) {
		alert('Please select a time');
		return;
	}
	
	// Send toggle schedule to server
	fetch(`${LS_SERVER_BASE_URL}/set-toggle-schedule`, {
		method: 'POST',
		headers: {
			'Content-Type': 'application/json'
		},
		body: JSON.stringify({
			light: light,
			time: timeValue,
			oneShot: true
		})
	})
	.then(res => res.json())
	.then(data => {
		if (data.success) {
			toggleTimes[light] = timeValue;
			alert(`Light ${light} will toggle at ${timeValue}`);
			console.log(`Toggle schedule for light ${light} set to ${timeValue}`);
		} else {
			alert('Failed to set toggle schedule: ' + (data.error || 'Unknown error'));
		}
	})
	.catch(err => {
		alert('Error setting toggle schedule: ' + err);
	});
}

// Update current time in TimeLog
function updateCurrentTime() {
	const now = new Date();
	const daysOfWeek = ['Sunday', 'Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday'];
	const year = now.getFullYear();
	const month = String(now.getMonth() + 1).padStart(2, '0');
	const day = String(now.getDate()).padStart(2, '0');
	const dayName = daysOfWeek[now.getDay()];
	const hours = String(now.getHours()).padStart(2, '0');
	const minutes = String(now.getMinutes()).padStart(2, '0');
	const seconds = String(now.getSeconds()).padStart(2, '0');
	
	const timeString = `${year}-${month}-${day} ${dayName} ${hours}:${minutes}:${seconds}`;
	document.getElementById('CT').textContent = `Current Time: ${timeString}`;
}

// Fetch RTC time from server
function fetchRTCTime() {
	// Send request to server to get RTC time
	// This assumes the server has an endpoint like /rtc or /time
	fetch(`${LS_SERVER_BASE_URL}/rtc`)
		.then(res => res.json())
		.then(data => {
			document.getElementById('RTC').textContent = `Server Time: ${data.time}`;
		})
		.catch(() => {
			document.getElementById('RTC').textContent = 'Server Time: Unavailable';
		});
}

// Sync RTC time with browser's current time
function syncRTCTime() {
	const now = new Date();
	const year = now.getFullYear();
	const month = now.getMonth() + 1;
	const day = now.getDate();
	const hours = now.getHours();
	const minutes = now.getMinutes();
	const seconds = now.getSeconds();
	
	// Send time to server via POST
	fetch(`${LS_SERVER_BASE_URL}/set-time`, {
		method: 'POST',
		headers: {
			'Content-Type': 'application/json'
		},
		body: JSON.stringify({
			year: year,
			month: month,
			day: day,
			hour: hours,
			minute: minutes,
			second: seconds
		})
	})
	.then(res => res.json())
	.then(data => {
		if (data.success) {
			alert('RTC time synced successfully!');
			fetchRTCTime(); // Refresh the displayed server time
		} else {
			alert('Failed to sync RTC time: ' + (data.error || 'Unknown error'));
		}
	})
	.catch(err => {
		alert('Error syncing RTC time: ' + err);
	});
}

function fetchUltrasonic() {
	fetch(`${LS_SERVER_BASE_URL}/ultrasonic`)
		.then(res => res.json())
		.then(data => {
			document.getElementById('ultrasonic-value').textContent = `${data.distance} cm`;
		})
		.catch(() => {
			document.getElementById('ultrasonic-value').textContent = '-- cm';
		});
}

function fetchPir() {
	fetch(`${LS_SERVER_BASE_URL}/pir`)
		.then(res => res.json())
		.then(data => {
			document.getElementById('pir-value').textContent = data.motion ? 'Motion Detected' : 'No Motion';
		})
		.catch(() => {
			document.getElementById('pir-value').textContent = 'No Motion';
		});
}

window.onload = () => {
	// Set default state to OFF for all bulbs and sliders
	for (let i = 1; i <= 3; i++) {
		updateStatus(i, 'off');
	}
	// Set default PIR sensor state to 'No Motion'
	document.getElementById('pir-value').textContent = 'No Motion';
	
	// Initialize time logging
	updateCurrentTime();
	fetchRTCTime();
	
	fetchStatus();
	fetchUltrasonic();
	fetchPir();
	// Optionally refresh status and sensors every 5 seconds
	setInterval(() => {
		updateCurrentTime();
		fetchRTCTime();
		fetchStatus();
		fetchUltrasonic();
		fetchPir();
	}, 5000);
};
