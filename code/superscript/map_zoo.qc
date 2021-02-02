// map_zoo.qc
//

.float message_count;
.entity spawned_army_entity; 

// This is set by the "script_spawn" attribute in worldspawn.
// And its called at the end of ClientBegin.

// Here we are overriding the save/load values, and initial spawn values for the player,
// and setting our own. This can be useful when setting up a map like e1m3 in Duke3D,
// were the player was on death row, and lost all his weapons.
// In our example, we are just setting the health to 75 and giving the player some armor.
void() map_zoo_initial_spawn
{
	SetEntityHealth(self, 75); // Player has less health at the start of our map.
	SetPlayerArmor(self, 50); // GIve the player some armor.
};

void() map_zoo_worldspawn
{
	// Since we are spawning in monsters that aren't placed in the level, we need to preache the data.
	monster_zombie_precache(null);
	monster_army_precache(null);
};

// This gets called by a trigger entity that had the script keyword set to "map_zoo_text_example"
void() map_zoo_text_example
{
	if(message_count == 0)
	{
		CenterPrint(self.activator, "Welcome to the zoo!");
	}
	message_count = 1;
};

// Spawn a new entity when the player hits the trigger, but only do so in certain circumstances.
void() map_zoo_spawn_army_test
{
	if(self.target_ent == null)
	{
		self.target_ent = FindEntity(self.target);

		if(self.target_ent == null)
		{
			Print("No target set, can't spawn anything!");
			return;
		}
	}

	if(spawned_army_entity == null)
	{
		spawned_army_entity = SpawnEntityByClass("monster_army", self.target_ent.origin, 180);
	}
	else
	{
		// Only respawn a new enemy if the last one was killed.
		if(spawned_army_entity.health <= 0)
		{
			FreeEntity(spawned_army_entity);
			spawned_army_entity = SpawnEntityByClass("monster_army", self.target_ent.origin, 180);
		}
	} 
};