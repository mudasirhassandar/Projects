import { initializeApp } from "https://www.gstatic.com/firebasejs/12.15.0/firebase-app.js";

import {
  getDatabase,
  ref,
  onValue,
} from "https://www.gstatic.com/firebasejs/12.15.0/firebase-database.js";

console.log("JS Connected");
emailjs.init("x4v8sQMVM710JyCYh");

// SMS Function
window.sendReport = function () {
  if (!window.milkData) {
    alert("Data not loaded yet");
    return;
  }

  let email = document.getElementById("num").value;

  if (email == "") {
    alert("Enter Email");
    return;
  }

  emailjs
    .send("service_m1i2dqv", "template_dwez4x9", {
      to_email: email,
      name: "Customer",
      fat: milkData.fat,
      snf: milkData.snf,
      water: milkData.water,
      temperature: milkData.temperature,
      price: milkData.price,
    })
    .then(
      function () {
        alert("Report Sent Successfully");
      },
      function (error) {
        alert("Email Failed");

        console.log(error);
      },
    );
};

// History Button
function historyData() {
  alert("Opening complete milk history...");
}

// Firebase Configuration

const firebaseConfig = {
  apiKey: "AIzaSyDwunyUMUm4QuqGhY5RUcbVqJJ2ZiPq50g",

  authDomain: "milk-analyzer-8d50e.firebaseapp.com",

  databaseURL:
    "https://milk-analyzer-8d50e-default-rtdb.asia-southeast1.firebasedatabase.app",

  projectId: "milk-analyzer-8d50e",

  storageBucket: "milk-analyzer-8d50e.firebasestorage.app",

  messagingSenderId: "1082368700739",

  appId: "1:1082368700739:web:984e0433ec635928cdbff6",
};

const app = initializeApp(firebaseConfig);

const db = getDatabase(app);

console.log("Firebase Connected");

// Read Firebase Data

onValue(
  ref(db, "readings"),
  (snapshot) => {
    let data = snapshot.val();

    console.log("Firebase Data:", data);
    window.milkData = data;

    if (data) {
      document.querySelector(".blue h1").innerHTML = data.fat + " %";

      document.querySelector(".green h1").innerHTML = data.snf + " %";

      document.querySelector(".yellow h1").innerHTML = data.water;

      document.querySelector(".purple h1").innerHTML = data.temperature + " °C";

      document.querySelector(".red h1").innerHTML = "₹ " + data.price;
    }
  },
  (error) => {
    console.log("Firebase Error:", error);
  },
);
