// Shared base URL configuration for your C backend server
const API_URL = "http://localhost:3000/api/data";

/**
 * Fetches an individual key-value item from the C server cache
 * and renders it directly inside the DOM element.
 */
async function getData() {
    const content = document.getElementById("api");

    try {
        // Querying for the specific key named "estado"
        const response = await fetch(`${API_URL}?key=estado`);
        
        if (!response.ok) {
            throw new Error(`HTTP error! Status: ${response.status}`);
        }

        const data = await response.json();

        // Dynamically rendering the exact properties returned by your C struct
        content.innerHTML = `
            <span><strong>KEY:</strong> ${data.key}</span><br>
            <span><strong>VALUE:</strong> ${data.value}</span>
        `;
    } catch (error) {
        content.textContent = "Error: Failed to load data from server.";
        console.error("Fetch operation failed:", error);
    }
}

/**
 * Sends a key-value pair with an optional Time-To-Live (TTL) 
 * to the C server cache using URL-encoded form data.
 */
async function saveData(key, value, ttlSeconds = 0) {
    try {
        // Construct the body structure into a "key=value" URL string format
        let formBody = `key=${encodeURIComponent(key)}&value=${encodeURIComponent(value)}`;
        
        // Append the optional TTL parameter if provided
        if (ttlSeconds > 0) {
            formBody += `&ttl=${ttlSeconds}`;
        }

        const response = await fetch(API_URL, {
            method: "POST",
            headers: {
                "Content-Type": "application/x-www-form-urlencoded"
            },
            body: formBody
        });

        if (!response.ok) {
            throw new Error(`HTTP error! Status: ${response.status}`);
        }

        const result = await response.json();
        console.log("Successfully saved data in C cache server!", result);
        return result;

    } catch (error) {
        console.error("Failed to save data into server cache:", error);
    }
}


getData();


