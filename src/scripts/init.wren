import "random" for Random

foreign class Player {
	foreign static getPos
	foreign static setPos=(vec3List)
}

class Main {
	static init() {
		System.print("Hi!")
	}

	static update(delta) {
		var pos = Player.getPos
		pos[2] = pos[2] + delta
		Player.setPos = pos
		System.print("delta: %(delta)")
		System.print(Player.getPos)
	}

	static cleanup() {
		System.print("Cleanup!")
	}
}
