export function open(node: HTMLElement, open: boolean) {
  const isOpenClass = "modal-is-open";

  const openDialog = (open: boolean) => {
    if (open) {
      document.documentElement.classList.add(isOpenClass);
      node.setAttribute("open", "true");
    } else {
      document.documentElement.classList.remove(isOpenClass);
      node.removeAttribute("open");
    }
  };

  openDialog(open);

  return { update: openDialog };
}
