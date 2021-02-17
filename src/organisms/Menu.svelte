<script lang="ts">
  import { push } from "svelte-spa-router";
  import { List } from "smelte";

  const menuItems = [
    {
      id: "wifi",
      value: "/wifi",
      text: "Wi-Fi Setup",
      icon: "wifi",
      subheading: "Change STA/AP mode, edit SSID/Password",
    },
    {
      id: "upload",
      value: "/upload",
      text: "Photo Upload",
      icon: "photo",
      subheading: "Upload and Edit Photo",
    },
    {
      id: "files",
      value: "/files",
      text: "File Management",
      icon: "folder",
      subheading: "Hide or Delete photos and change the display order",
    },
    {
      id: "control",
      value: "/control",
      text: "Display Control",
      icon: "gamepad",
      subheading:
        "Set display rotation, refresh interval and display are margins",
    },
    {
      id: "settings",
      value: "/settings",
      text: "Settings",
      icon: "settings",
      subheading: "Date/Time, Location and settings other than display",
    },
  ];

  const selectListItem = (e: { detail: string }) => {
    const menuItem = menuItems.find(({ id }) => id == e.detail);
    push(menuItem.value);
  };

  let clazz: string;
  export { clazz as class };
</script>

<section class="flex md:w-1/2 sm:w-auto m-auto {clazz || ''}">
  <List on:change={selectListItem} items={menuItems} class="w-full" />
</section>

<style>
</style>
