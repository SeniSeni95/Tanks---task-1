void handle_cell_collisions() {
    for (cell* c : board->collisions) { // Iterate over pointers to cells in the collisions vector
        if (c->objects.size() >= 2) { // Check for cells with at least 2 objects
            cout << "Collision detected in cell (" << c->x << ", " << c->y << ")!" << endl;

            // Print all objects in the cell
        // cout << "Objects in the cell:" << endl;
        // for (game_object* obj : c->objects) {
        //     cout << " - Object Symbol: " << obj->symbol << ", Position: (" << obj->x << ", " << obj->y << ")" << endl;
        // }
            game_object* first_obj = c->objects[0]; // Get the first object in the cell

            if (mine* m = dynamic_cast<mine*>(first_obj)) {
                // If the first object is a mine, destroy the mine and all tanks
                cout << "Mine detected in collision!" << endl;
                for (auto it = c->objects.begin(); it != c->objects.end();) {
                    game_object* obj = *it;
                    if (tank* t = dynamic_cast<tank*>(obj)) {
                        cout << "Destroying tank " << t->symbol << " due to mine!" << endl;
                        board->remove_tank(t);
                        delete t;
                        c->remove_Object(obj); // This will remove it from the vector
                        it = c->objects.begin(); // Reset iterator because vector changed
                    } else {
                        ++it;
                    }
                }
                // Destroy the mine
                // cout <<"here!"<<endl;
                c->remove_Object(m);
                // cout <<"remove_object"<<endl;
                delete m; // Delete the mine
                // cout <<"delete_m"<<endl;
                game_over = true; // End the game
            } else if (shell* s = dynamic_cast<shell*>(first_obj)) {
                // If the first object is a shell, destroy the shell and all tanks
                cout << "Shell detected in collision!" << endl;
                for (auto it = c->objects.begin(); it != c->objects.end();) {
                    game_object* obj = *it;
                    if (tank* t = dynamic_cast<tank*>(obj)) {
                        cout << "Destroying tank " << t->symbol << " due to shell!" << endl;
                        c->remove_Object(t);
                        board->remove_tank(t);
                        delete t; // Delete the tank
                        it = c->objects.erase(it); // Safely remove the tank from the objects vector
                    } else {
                        ++it; // Skip other objects (e.g., mines)
                    }
                }
                // Destroy the shell
                c->remove_Object(s);
                board->remove_shell(s);
                delete s; // Delete the shell
                game_over = true; // End the game
            } else if (tank* t = dynamic_cast<tank*>(first_obj)) {
                // If the first object is a tank, destroy everything in the cell
                cout << "Tank detected in collision!" << endl;
                for (auto it = c->objects.begin(); it != c->objects.end();) {
                    game_object* obj = *it;
                    if (dynamic_cast<shell*>(obj) || dynamic_cast<mine*>(obj)) {
                        ++it; // Skip shells and mines
                    } else {
                        cout << "Destroying object with symbol: " << obj->symbol << endl;
                        c->remove_Object(obj);
                        delete obj; // Delete the object
                        it = c->objects.erase(it); // Safely remove the object from the objects vector
                    }
                }
                game_over = true; // End the game
            }
        }
    }

    // Clear the collisions vector after handling all collisions
    board->collisions.clear();
}
