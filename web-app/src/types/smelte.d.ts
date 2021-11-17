declare module "smelte/src/dark" {
  import { Writable } from "svelte/store";
  export default (): Writable<boolean> => {};
}
declare module "smelte" {
  import { SvelteTypedComponent } from "svelte-typed-component";
  export { default as Button } from "smelte/src/components/Button";
  export { default as Card } from "smelte/src/components/Card";
  export { default as Chip } from "smelte/src/components/Chip";
  export { default as Dialog } from "smelte/src/components/Dialog";
  export { default as Image } from "smelte/src/components/Image";
  export { default as List } from "smelte/src/components/List";
  export {
    default as ListItem,
    ListItemProps,
  } from "smelte/src/components/List/ListItem";
  export { default as Icon } from "smelte/src/components/Icon";
  export { default as Menu } from "smelte/src/components/Menu";
  export { default as NavigationDrawer } from "smelte/src/components/NavigationDrawer";
  export {
    default as Checkbox,
    Label as SelectionLabel,
  } from "smelte/src/components/Checkbox";
  export {
    default as RadioButtonGroup,
    RadioButton,
  } from "smelte/src/components/RadioButton";
  export { default as AppBar } from "smelte/src/components/AppBar";
  export { default as Select } from "smelte/src/components/Select";
  export { default as ProgressLinear } from "smelte/src/components/ProgressLinear";
  export { default as ProgressCircular } from "smelte/src/components/ProgressCircular";
  export { default as Slider } from "smelte/src/components/Slider";
  export {
    default as Snackbar,
    notifier,
    Notifications,
  } from "smelte/src/components/Snackbar";
  export { TabButton, Tabs } from "smelte/src/components/Tabs";
  import { TabEvents, TabSlots } from "smelte/src/components/Tabs/Tab";
  export class Tab extends SvelteTypedComponent<
    { selected?: string | boolean; id?: string | null },
    TabEvents,
    TabSlots
  > {}
  export { default as TextField, Label } from "smelte/src/components/TextField";
  export { default as DataTable } from "smelte/src/components/DataTable";
  export { default as Switch } from "smelte/src/components/Switch";
  export { default as Tooltip } from "smelte/src/components/Tooltip";
  export { default as Treeview } from "smelte/src/components/Treeview";
  export { default as Ripple } from "smelte/src/components/Ripple";
  export { default as DatePicker } from "smelte/src/components/DatePicker";
  export { default as breakpoints } from "smelte/src/breakpoints";
  export { default as notificationQueue } from "smelte/src/notification-queue";
  export { Scrim, Spacer } from "smelte/src/components/Util";
}
