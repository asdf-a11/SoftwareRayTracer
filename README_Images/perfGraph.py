import matplotlib.pyplot as plt

def draw_bar_chart(values, names, title="Bar Chart", ylabel="Value"):
    #plt.figure(figsize=(10, 6))
    plt.bar(names, values, color='gray')
    plt.title(title)
    plt.ylabel(ylabel)
    plt.xlabel("Optimizations")
    plt.tight_layout()
    plt.show()


if __name__ == "__main__":
    #In seconds starting from 6
    values = [
        6.176,    
        5.934,
        5.733,
        4.9,
        4.782,
        4.782*0.984,
        2.13/2.536*4.545
    ]
    names = [
        "BVH",
        "Remove normalization",
        "Faster sorting for bounding boxes",
        "Reduce heap allocations",
        "Custom sin function",
        "Combining faces into one",
        "Dynamic spacial splits"
    ]
    draw_bar_chart(values, names, title="Seconds per frame", ylabel="Seconds")