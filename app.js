import { initializeApp } from "https://www.gstatic.com/firebasejs/11.0.2/firebase-app.js";
import { getDatabase, ref, onValue } from "https://www.gstatic.com/firebasejs/11.0.2/firebase-database.js";

// Firebase configuration
const firebaseConfig = {
  apiKey: "AIzaSyBgTl78i1F7QHE7dPWEu3PlgQm_52piBv4",
  authDomain: "iot-database-00013781.firebaseapp.com",
  databaseURL: "https://iot-database-00013781-default-rtdb.firebaseio.com",
  projectId: "iot-database-00013781",
  storageBucket: "iot-database-00013781.appspot.com",
  messagingSenderId: "511611588776",
  appId: "1:511611588776:web:f111fec4169dfdb128729d"
};

// Initialize Firebase
const app = initializeApp(firebaseConfig);
const database = getDatabase(app);

// DOM Elements
const leaderboardBody = document.getElementById('leaderboard');

// Function to fetch and display leaderboard data
function fetchLeaderboardData() {
  const leaderboardRef = ref(database, 'MemoryGame/Players');
  onValue(leaderboardRef, (snapshot) => {
    const data = snapshot.val();
    leaderboardBody.innerHTML = ''; // Clear previous leaderboard data

    if (data) {
      for (const playerID in data) {
        const player = data[playerID];

        // Ensure that each data item has the expected fields
        const playerLevel = player.Level || 'N/A';
        const playerScore = player.Score || 'N/A';
        const playerTime = player.Time || 'N/A'; // Displaying time directly from Firebase

        // Append player data to leaderboard
        const row = `
          <tr>
            <td>${playerLevel}</td>
            <td>${playerScore}</td>
            <td>${playerTime}</td>
          </tr>
        `;
        leaderboardBody.innerHTML += row;
      }
    } else {
      leaderboardBody.innerHTML = '<tr><td colspan="3">No data available</td></tr>';
    }
  });
}

// Call the fetchLeaderboardData function to display the leaderboard when the page loads
fetchLeaderboardData();
