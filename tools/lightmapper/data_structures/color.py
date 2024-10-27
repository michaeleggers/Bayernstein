class Color:
    def __init__(self, r, g, b):
        self.r = r
        self.g = g
        self.b = b

    def __repr__(self):
        return f"Color([{self.r}, {self.g}, {self.b}])"

    def __str__(self):
        return f"[{self.r}, {self.g}, {self.b}]"
    
    def sum(self) -> float:
        return self.r + self.g + self.b