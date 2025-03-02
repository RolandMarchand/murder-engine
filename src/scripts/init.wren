// Foreign classes act as game context for [Main.update()]
foreign class Player {
	foreign static getPos
	foreign static setPos=(vec3List)
}

foreign class Vec3 {
	foreign construct new(x, y, z)

	foreign [index]
	foreign [index]=(num)
	foreign set(x, y, z)
}

class Main {
	// Ran only once before the first frame
	static init() {
		System.print("Hi!")

		// Test bindings
		var v = Vec3.new(10, 20, 30)
		v[2] = 300
		v.set(2, 2, 2)
		System.print("Vector size: %(v[0]), %(v[1]), %(v[2])")
	}

	// Ran every frame
	static update(delta) {
		// Test stuff
		// var pos = Player.getPos
		// pos[1] = pos[1] - delta
		// Player.setPos = pos
		System.print("delta: %(delta)")
		System.print(Player.getPos)
	}

	// Ran only once after the last frame
	static cleanup() {
		System.print("Cleanup!")
	}
}
