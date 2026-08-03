#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <map>
#include "../include/graph/TaskGraph.h"
#include "../include/concurrency/ThreadPool.h"
#include "../include/concurrency/Scheduler.h"
#include "../include/tasks/FetchTask.h"
#include "../include/tasks/PackTask.h"

using namespace std;

// A simple structure to represent our predefined catalog
struct CatalogItem {
    string name;
    string location;
};

int main() {
    cout << "===========================================\n";
    cout << "  Concurrent Warehouse Task Scheduler CLI  \n";
    cout << "===========================================\n\n";

    // 1. Define our fixed set of catalog items
    map<int, CatalogItem> catalog = {
        {1, {"Smartphone", "Aisle 1 - Electronics"}},
        {2, {"Laptop", "Aisle 2 - Electronics"}},
        {3, {"Headphones", "Aisle 3 - Audio"}},
        {4, {"Coffee Mug", "Aisle 4 - HomeGoods"}},
        {5, {"Notebook", "Aisle 5 - Office"}}
    };

    int num_orders = 0;
    cout << "How many orders would you like to process today? ";
    cin >> num_orders;

    if (num_orders <= 0) {
        cout << "No orders to process. Exiting.\n";
        return 0;
    }

    TaskGraph graph;
    int task_id_counter = 1; // Global counter to ensure every task has a unique ID
    int total_tasks_created = 0;
    vector<int> all_pack_task_ids; // We need this to link them to the final ShipTask

    // 2. Dynamically build the graph based on user input
    for (int i = 1; i <= num_orders; ++i) {
        cout << "\n--- Creating Order #" << i << " ---\n";
        cout << "Available Catalog:\n";
        for (const auto& [id, item] : catalog) {
            cout << "  [" << id << "] " << item.name << " (" << item.location << ")\n";
        }

        int num_items;
        cout << "How many items in Order #" << i << "? ";
        cin >> num_items;

        vector<int> current_order_fetch_ids;

        // Generate FetchTasks for this order
        for (int j = 1; j <= num_items; ++j) {
            int choice;
            cout << "Enter Catalog ID for item " << j << ": ";
            cin >> choice;

            // Fallback if user enters a bad ID
            if (catalog.find(choice) == catalog.end()) {
                cout << "Invalid choice. Defaulting to Smartphone.\n";
                choice = 1;
            }

            CatalogItem selected = catalog[choice];
            int current_fetch_id = task_id_counter++;
            
            auto fetch_task = make_shared<FetchTask>(
                current_fetch_id, 
                "Fetch " + selected.name, 
                1, // Priority 1
                selected.location
            );
            
            graph.addTask(fetch_task);
            current_order_fetch_ids.push_back(current_fetch_id);
            total_tasks_created++;
        }

        // Generate a single PackTask for this order
        int current_pack_id = task_id_counter++;
        string order_str = "ORD-2026-" + to_string(i);
        auto pack_task = make_shared<PackTask>(
            current_pack_id, 
            "Pack Order " + to_string(i), 
            2, // Priority 2
            order_str
        );
        
        graph.addTask(pack_task);
        all_pack_task_ids.push_back(current_pack_id);
        total_tasks_created++;

        // Link the FetchTasks to this PackTask
        for (int fetch_id : current_order_fetch_ids) {
            graph.addDependency(fetch_id, current_pack_id);
        }
    }

    // 3. Create the final ShipTask that waits for ALL PackTasks
    cout << "\n[System] Compiling dispatch dependencies...\n";
    int ship_task_id = task_id_counter++;
    auto ship_task = make_shared<FetchTask>(
        ship_task_id, 
        "Load All Orders to Dispatch Truck", 
        3, 
        "Loading Bay A"
    );
    
    graph.addTask(ship_task);
    total_tasks_created++;

    for (int pack_id : all_pack_task_ids) {
        graph.addDependency(pack_id, ship_task_id);
    }

    // 4. Execute the dynamic graph
    cout << "\n===========================================\n";
    cout << "  Starting Warehouse Execution Engine      \n";
    cout << "  Total Tasks Generated: " << total_tasks_created << "\n";
    cout << "===========================================\n";

    ThreadPool pool(3); 
    Scheduler scheduler(graph, pool, total_tasks_created);
    scheduler.start();

    return 0;
}