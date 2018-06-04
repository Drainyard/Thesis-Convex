import matplotlib
import os
from matplotlib import rcParams
rcParams['font.family'] = 'serif'
rcParams['font.sans-serif'] = ['Palatino']
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.ticker import FormatStrFormatter

def format_num():
    return (lambda x, p: x)
    
def format_axis(ax):
    ax.get_xaxis().set_major_formatter(
        matplotlib.ticker.FuncFormatter(lambda x, p: format(int(x), ','))
    )

    ax.yaxis.set_major_formatter
    (
        matplotlib.ticker.FuncFormatter(format_num())
    )

def plot_single(path, title, outfile, label, ylabel, show=False):
    data = np.genfromtxt(path, delimiter=',', names=['x', 'y'])

    data['y'] = [np.divide(y, 1000000.0) for y in data['y']]

    fig, ax = plt.subplots()
    ax.plot(data['x'], data['y'], color='black', label=label, marker='x', markersize=6.0, linewidth=0.5)
    format_axis(ax)
    ax.legend()

    plt.xlabel('n')
    plt.ylabel(ylabel)
    plt.title(title)
    savefig(outfile)
    if show:
        plt.show()

    plt.close()


def plot_comparison(path, title, outfile, ylabel, labels = ['QuickHull', 'Incremental'], divide=True, yrange=None, minrange=0, percent=False, show=False):
    data = np.genfromtxt(path, delimiter=',', names=['x_qh', 'y_qh', 'x_inc', 'y_inc'])

    fig, ax = plt.subplots()

    if divide:
        data['y_qh'] = [np.divide(y, 1000000.0) for y in data['y_qh']]
        data['y_inc'] = [np.divide(y, 1000000.0) for y in data['y_inc']]

    if labels != None:
        ax.plot(data['x_qh'][minrange:,], data['y_qh'][minrange:,], color='black', label=labels[0], marker='x', markersize=6.0, linewidth=0.5)
        ax.plot(data['x_inc'][minrange:,], data['y_inc'][minrange:,], color='black', label=labels[1], marker='^', markersize=6.0, linewidth=0.5, linestyle='dashed')
        format_axis(ax)
        ax.legend()
    else:
        ax.plot(data['x_qh'][minrange:,], data['y_qh'][minrange:,], color='black', marker='x', markersize=6.0, linewidth=0.5)
        ax.plot(data['x_inc'][minrange:,], data['y_inc'][minrange:,], color='black', marker='^', markersize=6.0, linewidth=0.5, linestyle='dashed')
        format_axis(ax)

    if percent:
        vals = ax.get_yticks()
        ax.set_yticklabels(['{:3.2f}%'.format(x) for x in vals])
    
    if yrange != None:
         ax.set_ylim(yrange)

    plt.xlabel('n')
    plt.ylabel(ylabel)
    plt.title(title)
    savefig(outfile)
    if show:
        plt.show()

    plt.close()

def plot_three(path, title, outfile, ylabel, labels=['QuickHull', 'Incremental', 'Points on hull'], divide=False, show=False):
    data = np.genfromtxt(path, delimiter=',', names=['x_qh', 'y_qh', 'x_inc', 'y_inc', 'x_out', 'y_out'])

    fig, ax = plt.subplots()

    if divide:
        data['y_qh'] = [np.divide(y, 1000000.0) for y in data['y_qh']]
        data['y_inc'] = [np.divide(y, 1000000.0) for y in data['y_inc']]
        data['y_out'] = [np.divide(y, 1000000.0) for y in data['y_out']]

    if labels != None:
        ax.plot(data['x_qh'], data['y_qh'], color='black', label=labels[0], marker='x', markersize=6.0, linewidth=0.5)
        ax.plot(data['x_inc'], data['y_inc'], color='black', label=labels[1], marker='^', markersize=6.0, linewidth=0.5, linestyle='dashed')
        ax.plot(data['x_out'], data['y_out'], color='black', label=labels[2], marker='p', markersize=6.0, linewidth=0.5, linestyle='dashdot')
        format_axis(ax)
        ax.legend()
    else:
        ax.plot(data['x_qh'], data['y_qh'], color='black', marker='x', markersize=6.0, linewidth=0.5)
        ax.plot(data['x_inc'], data['y_inc'], color='black', marker='^', markersize=6.0, linewidth=0.5, linestyle='dashed')
        ax.plot(data['x_out'], data['y_out'], color='black', marker='p', markersize=6.0, linewidth=0.5, linestyle='dashdot')
        format_axis(ax)                    

    plt.xlabel('n')
    plt.ylabel(ylabel)
    plt.title(title)
    savefig(outfile)
    if show:
        plt.show()

    plt.close()

def plot_single_nlogn(path, title, outfile, label, miny = [0.0, 0.5], minrange=0, show=False):
    data = np.genfromtxt(path, delimiter=',', names=['x', 'y'])

    fig, ax = plt.subplots()

    data['y'] = [np.divide(y, np.multiply(x, np.log2(x))) for y,x in zip(data['y'], data['x'])]
    ax.plot(data['x'][minrange:,], data['y'][minrange:,], color='black', label=label, marker='x', markersize=6.0, linewidth=0.5)
    #ax.plot(data['x_inc'], data['y_inc'], color='black', label='data', marker='^', markersize=6.0)
    format_axis(ax)

    ax.set_ylim(miny)
    ax.legend()

    plt.xlabel('n')
    plt.ylabel('Time spent over n log n (s)')
    plt.title(title)
    savefig(outfile)
    if show:
        plt.show()

    plt.close()

def plot_nlogn(path, title, outfile, miny = [0.0, 0.5], minrange=0, show=False):
    data = np.genfromtxt(path, delimiter=',', names=['x_qh', 'y_qh', 'x_inc', 'y_inc'])

    fig, ax = plt.subplots()

    data['y_qh'] = [np.divide(y, np.multiply(x, np.log2(x))) for y,x in zip(data['y_qh'], data['x_qh'])]
    data['y_inc'] = [np.divide(y, np.multiply(x, np.log2(x))) for y,x in zip(data['y_inc'], data['x_inc'])]
    ax.plot(data['x_qh'][minrange:,], data['y_qh'][minrange:,], color='black', label='QuickHull', marker='x', markersize=6.0, linewidth=0.5)
    ax.plot(data['x_inc'][minrange:,], data['y_inc'][minrange:,], color='black', label='Incremental', marker='^', markersize=6.0, linewidth=0.5, linestyle='dashed')
    #ax.plot(data['x_inc'], data['y_inc'], color='black', label='data', marker='^', markersize=6.0)
    format_axis(ax)

    ax.set_ylim(miny)
    ax.legend()

    plt.xlabel('n')
    plt.ylabel('Time spent over n log n (s)')
    plt.title(title)
    savefig(outfile)
    if show:
        plt.show()

    plt.close()

def plot_nsquared(path, title, outfile, miny = [0.0, 0.5], minrange=0, show=False):
    data = np.genfromtxt(path, delimiter=',', names=['x_qh', 'y_qh', 'x_inc', 'y_inc'])

    fig, ax = plt.subplots()

    data['y_qh'] = [np.divide(y, np.multiply(x, x)) for y,x in zip(data['y_qh'], data['x_qh'])]
    data['y_inc'] = [np.divide(y, np.multiply(x, x)) for y,x in zip(data['y_inc'], data['x_inc'])]
    ax.plot(data['x_qh'][minrange:,], data['y_qh'][minrange:,], color='black', label='QuickHull', marker='x', markersize=6.0, linewidth=0.5)
    ax.plot(data['x_inc'][minrange:,], data['y_inc'][minrange:,], color='black', label='Incremental', marker='^', markersize=6.0, linewidth=0.5, linestyle='dashed')
    #ax.plot(data['x_inc'], data['y_inc'], color='black', label='data', marker='^', markersize=6.0)
    format_axis(ax)

    ax.set_ylim(miny)
    ax.legend()

    plt.xlabel('n')
    plt.ylabel('Time spent over n log n (s)')
    plt.title(title)
    savefig(outfile)
    
    if show:
        plt.show()

    plt.close()

def savefig(path):
    plt.savefig(os.path.join('graphs' + os.sep, path + '.pdf'), format='pdf', dpi=1000)
    

if __name__ == '__main__':
    basedir = 'c:\\Users\\Niels\\projects\\Thesis-Convex\\data\\'
    plot_comparison(os.path.join(basedir, 'in_sphere_10m.csv'), 'Time for points in a sphere', 'time_qh_inc_in_sphere', 'Time spent (s)')
    plot_nlogn(os.path.join(basedir, 'in_sphere_10m.csv'), 'Time for points in a sphere over nlogn', 'time_qh_inc_in_sphere_nlogn', minrange=8)
    plot_comparison(os.path.join(basedir, 'on_sphere_100k.csv'), 'Time for points on a sphere', 'time_qh_inc_on_sphere', 'Time spent (s)')
    plot_nlogn(os.path.join(basedir, 'on_sphere_100k.csv'), 'Time for points on a sphere over nlogn', 'time_qh_inc_on_sphere_nlogn', [0.0,10.5])
    plot_nsquared(os.path.join(basedir, 'on_sphere_100k.csv'), 'Time for points on a sphere over nsquared', 'time_qh_inc_on_sphere_squard', [0.0,0.01], 5)
    plot_comparison(os.path.join(basedir, 'in_cube_1_2m.csv'), 'Time for points in a cube', 'time_qh_inc_in_cube', 'Time spent (s)')
    plot_nlogn(os.path.join(basedir, 'in_cube_1_2m.csv'), 'Time for points in a cube over nlogn', 'time_qh_inc_in_cube_nlogn', [0.0, 1.5], 7)
    plot_comparison(os.path.join(basedir, 'normalized_sphere.csv'), 'Time for points on a normalized sphere', 'time_qh_inc_normalized_sphere', 'Time spent (s)')
    plot_nsquared(os.path.join(basedir, 'normalized_sphere.csv'), 'Time for points on a normalized sphere over nsquared', 'time_qh_inc_normalized_squared', [0.0,0.02], 5)
    plot_comparison(os.path.join(basedir, 'many_internal.csv'), 'Time for points on a distribution with many internal points', 'time_qh_inc_many_internal', 'Time spent (s)')
    plot_single(os.path.join(basedir, 'qh_many_internal.csv'), 'Time for points on a distribution with many internal points', 'time_qh_many_internal', 'QuickHull', 'Time spent (s)')
    plot_single_nlogn(os.path.join(basedir, 'qh_many_internal.csv'), 'Time for points on a distribution with many internal points over nlogn', 'time_qh_many_internal_nlogn', 'QuickHull', [0.0, 0.15], 5)
    plot_comparison(os.path.join(basedir, 'on_sphere_processed.csv'), 'Number of points processed for points on a sphere', 'processed_qh_inc_on_sphere', 'Processed points')
    plot_three(os.path.join(basedir, 'many_internal_processed.csv'), 'Number of points processed and in hull when many points are internal', 'processed_internal', 'Points')
    plot_comparison(os.path.join(basedir, 'normalized_qh.csv'), 'Time on sphere and on normalized sphere (QuickHull)', 'time_qh_on_norm_sphere', 'Time spent (s)', labels=None)
    plot_three(os.path.join(basedir, 'in_sphere_dac_inc_qh.csv'), 'Time for points in a sphere', 'in_sphere_dac_inc_qh', 'Time spent (s)', ['QuickHull', 'Incremental', 'Divide and Conquer'], divide=True)
    plot_three(os.path.join(basedir, 'on_sphere_dac_inc_qh.csv'), 'Time for points on a sphere', 'on_sphere_dac_inc_qh', 'Time spent (s)', ['QuickHull', 'Incremental', 'Divide and Conquer'], divide=True)
    plot_comparison(os.path.join(basedir, 'on_sphere_inc_dac.csv'), 'Time for points on a sphere', 'on_sphere_inc_dac', 'Time spent (s)', labels=['Incremental', 'Divide and Conquer'])
    plot_three(os.path.join(basedir, 'in_cube_dac_inc_qh.csv'), 'Time for points in a cube', 'in_cube_dac_inc_qh', 'Time spent (s)', ['QuickHull', 'Incremental', 'Divide and Conquer'], divide=True)
    plot_comparison(os.path.join(basedir, 'faces_on_hull_in_sphere_qh_inc.csv'), 'Percentage of created faces on the hull', 'faces_on_hull_in_sphere', 'Faces on hull (%)', labels=['QuickHull', 'Incremental'], divide=False, yrange=[5.0, 25.0], minrange=5, percent=True)
    plot_comparison(os.path.join(basedir, 'faces_on_hull_normalized_sphere_qh_inc.csv'), 'Percentage of created faces on the hull', 'faces_on_hull_on_norm_sphere', 'Faces on hull (%)', labels=['QuickHull', 'Incremental'], divide=False, yrange=[32.0, 35.0], minrange=5, percent=True)
    plot_comparison(os.path.join(basedir, 'faces_on_hull_on_sphere_qh_inc.csv'), 'Percentage of created faces on the hull', 'faces_on_hull_on_sphere', 'Faces on hull (%)', labels=['QuickHull', 'Incremental'], divide=False, yrange=[32.5, 34.0], minrange=5, percent=True)
    