import streamlit as st
import firebase_admin
from firebase_admin import credentials, db
import geopy.distance
from datetime import datetime
import folium
from streamlit_folium import folium_static

if not firebase_admin._apps:
    cred = credentials.Certificate("firebase_config.json")
    firebase_admin.initialize_app(cred, {
        'databaseURL': 'databaseURL'
    })

# Database references
fire_trucks_ref = db.reference("fire_trucks")
truck_status_ref = db.reference("truck_status")
scan_history_ref = db.reference("scan_history")

def get_available_trucks():
    """Get list of trucks currently in station based on scan count"""
    trucks = truck_status_ref.get()
    available_trucks = []
    
    if trucks:
        for truck_id, data in trucks.items():
            # Even scan count means truck is in station
            if data.get('scan_count', 0) % 2 == 0:
                available_trucks.append({
                    'truck_id': truck_id,
                    'station_id': data.get('station_id'),
                    'last_updated': data.get('timestamp'),
                    'latitude': data.get('latitude'),
                    'longitude': data.get('longitude')
                })
    
    return available_trucks

def find_nearest_trucks(fire_location, required_trucks):
    """Find nearest available trucks that are in station"""
    available_trucks = get_available_trucks()
    trucks_with_distance = []
    
    for truck in available_trucks:
        truck_loc = (truck['latitude'], truck['longitude'])
        distance = geopy.distance.distance(fire_location, truck_loc).km
        trucks_with_distance.append({
            **truck,
            'distance': distance
        })
    
    # Sort by distance and return required number of trucks
    return sorted(trucks_with_distance, key=lambda x: x['distance'])[:required_trucks]

def create_map(fire_location, trucks):
    """Create a map showing fire location and truck positions"""
    m = folium.Map(location=fire_location, zoom_start=12)
    
    folium.Marker(
        fire_location,
        popup="Fire Location",
        icon=folium.Icon(color='red', icon='fire', prefix='fa')
    ).add_to(m)
    
    for truck in trucks:
        folium.Marker(
            (truck['latitude'], truck['longitude']),
            popup=f"Truck {truck['truck_id']} at {truck['station_id']}",
            icon=folium.Icon(color='green', icon='truck', prefix='fa')
        ).add_to(m)
    
    return m

st.title("🚒 Fire Brigade Dispatch System")

st.header("System Status")

st.subheader("Available Trucks")
available_trucks = get_available_trucks()
if available_trucks:
    for truck in available_trucks:
        st.success(f"🚒 Truck {truck['truck_id']} at {truck['station_id']}")
else:
    st.warning("No trucks currently available in stations")

st.header("Emergency Dispatch")
latitude = st.number_input("Fire Location Latitude", value=0.0, format="%.6f")
longitude = st.number_input("Fire Location Longitude", value=0.0, format="%.6f")
required_trucks = st.number_input("Required Fire Trucks", min_value=1, value=1, step=1)

if st.button("Dispatch Emergency Response"):
    fire_location = (latitude, longitude)
    selected_trucks = find_nearest_trucks(fire_location, required_trucks)
    
    if len(selected_trucks) >= required_trucks:
        st.success(f"✅ Found {len(selected_trucks)} available trucks!")
        
        m = create_map(fire_location, selected_trucks)
        folium_static(m)
        
        for truck in selected_trucks:
            st.write(f"🚒 Dispatching Truck {truck['truck_id']}")
            st.write(f"📍 Distance to location: {truck['distance']:.2f} km")
            st.write(f"🏠 From station: {truck['station_id']}")
    else:
        st.error(f"❌ Only {len(selected_trucks)} trucks available. Need {required_trucks}!")
        st.warning("Consider requesting backup from other districts.")

st.header("Truck Movement History")
selected_truck = st.selectbox(
    "Select Truck to View History",
    options=[truck_id for truck_id in (truck_status_ref.get() or {}).keys()]
)

if selected_truck:
    history = scan_history_ref.child(selected_truck).get() or {}
    st.write(f"Movement history for Truck {selected_truck}:")
    
    for timestamp, data in sorted(history.items(), key=lambda x: x[1]['timestamp']):
        status = "Entered Station" if data['scan_count'] % 2 == 0 else "Left Station"
        time = datetime.fromtimestamp(data['timestamp']/1000).strftime('%Y-%m-%d %H:%M:%S')
        st.write(f"⏰ {time}: {status} at {data['station_id']}")
