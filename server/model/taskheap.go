package model

type TaskHeap []*Task

func (t TaskHeap) Len() int {
	return len(t)
}

func (t TaskHeap) Less(i, j int) bool {
	return t[i].Priority < t[j].Priority
}

func (t TaskHeap) Swap(i, j int) {
	t[i], t[j] = t[j], t[i]
}

func (t *TaskHeap) Push(x any) {
	item := x.(*Task)
	*t = append(*t, item)
}

func (t *TaskHeap) Pop() any {
	old := *t
	n := len(old)
	if (n == 0) {
		return nil
	}

	item := old[n-1]
	old[n-1] = nil  // avoid memory leak
	*t = old[0 : n-1]
	return item
}
