# 🚒 RFID Fire Truck Dispatch System  

This project is an **RFID-based fire truck tracking and dispatch system** using **ESP32, Firebase, and Streamlit**. It helps fire stations **monitor truck availability** and **dispatch the nearest available trucks** during emergencies.  

---

## ✨ Features  
### 🔹 1. RFID-Based Fire Truck Tracking  
- Each fire truck is assigned an **RFID tag**.  
- The **ESP32 with an RFID reader** scans the tag when a truck enters or leaves the station.  
- A **scan count** determines if a truck is **available in the station** (even count) or **dispatched** (odd count).  
- The truck status is **updated in Firebase** in real-time.  

### 🔹 2. Real-Time Data Sync with Firebase  
- The system stores the **truck's status**, **scan history**, and **availability** in **Firebase Realtime Database**.  
- Each RFID scan automatically **updates Firebase**, ensuring **accurate truck tracking**.  
- Data remains accessible via the **Streamlit web app** for monitoring and decision-making.  

### 🔹 3. Smart Dispatch System (Nearest Trucks)  
- When a **fire location** is entered in the **web app**, the system:  
  1. Fetches all available trucks from Firebase.  
  2. **Calculates the distance** between the fire location and each truck's last known station.  
  3. **Sorts the trucks** by distance.  
  4. Selects the **required number of nearest trucks** for dispatch.  

### 🔹 4. Request Additional Trucks from Other Stations  
- If a fire location **needs more trucks than the local station has**, the system:  
  1. Identifies **other nearest fire stations** using their coordinates.  
  2. Checks **truck availability** at these stations.  
  3. Dispatches additional trucks from the **nearest station with available resources**.  

### 🔹 5. Interactive Map for Fire Truck & Fire Location  
- Uses **Folium** to create an **interactive map** in the web app.  
- Displays:  
  - 🚒 **Truck locations** (Green = Available, Orange = Dispatched).  
  - 🔥 **Fire location** (Red marker).  

### 🔹 6. Truck Entry & Exit Logging  
- Logs each **truck’s entry and exit** from the fire station.  
- Displays a **detailed timestamped history** in the **web app**.  
- Helps analyze **truck response time** and improve efficiency.  

---

## 🛠️ Tech Stack  
🔹 **ESP32 + RFID Module** – Reads truck tags and updates Firebase.  
🔹 **Firebase Realtime Database** – Stores truck availability, status, and history.  
🔹 **Streamlit + Folium** – Provides a **web dashboard** for monitoring and dispatching.  
🔹 **Geopy** – Calculates distances for optimal dispatch.  

---

## 📌 How It Works  

### 🚀 Step 1: Scanning Trucks at the Fire Station  
1️⃣ Each **truck has an RFID tag**.  
2️⃣ A **station-based RFID reader (ESP32)** scans the tag when a truck enters or leaves.  
3️⃣ If a truck **enters**, it's marked **"Available"**; if it **leaves**, it's marked **"Dispatched"**.  
4️⃣ The **Firebase Realtime Database updates** truck status automatically.  

### 🚀 Step 2: Fire Emergency Reported  
1️⃣ Firefighters **enter the fire location** (latitude/longitude) in the **Streamlit web app**.  
2️⃣ The system **fetches all available trucks** from Firebase.  
3️⃣ **Calculates the distance** between fire trucks and the fire location.  
4️⃣ **Selects the nearest trucks** for dispatch.  

### 🚀 Step 3: Dispatch & Request Additional Trucks  
1️⃣ Selected fire trucks are **marked as dispatched** in Firebase.  
2️⃣ If **more trucks are needed**, the system:  
   - **Finds nearby fire stations** with available trucks.  
   - **Requests additional trucks** based on distance.  
3️⃣ The system **logs the dispatch event** in Firebase.  
4️⃣ The **interactive map updates** with the truck and fire locations.  
5️⃣ The web app displays **truck entry/exit history** for analysis.  
