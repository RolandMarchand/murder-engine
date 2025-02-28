// Foreign classes act as game context for [Main.update()]
foreign class Player {
	foreign static getPos
	foreign static setPos=(vec3List)
}

class Main {
	// Ran only once before the first frame
	static init() {
		System.print("Hi!")
	}

	// Ran every frame
	static update(delta) {
		// Test stuff
		var pos = Player.getPos
		pos[2] = pos[2] + delta
		Player.setPos = pos
		System.print("delta: %(delta)")
		System.print(Player.getPos)
	}

	// Ran only once after the last frame
	static cleanup() {
		System.print("Cleanup!")
	}
}
