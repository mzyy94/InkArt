<script lang="ts">
  import { push } from "svelte-spa-router";
  import { List, Dialog, Button } from "smelte";
  import Container from "../templates/Container.svelte";

  const menuItems = [
    {
      id: "upload",
      value: "/upload",
      text: "Upload photo",
      icon: "photo",
      subheading: "Edit and upload photo",
    },
    {
      id: "files",
      value: "/files",
      text: "File Management",
      icon: "folder",
      subheading: "Organize photos to be shown",
    },
    {
      id: "control",
      value: "/control",
      text: "Display Control",
      icon: "gamepad",
      subheading: "Set display orientation and paddings",
    },
    {
      id: "settings",
      value: "/settings",
      text: "Settings",
      icon: "settings",
      subheading: "Change device Date/Time, refresh interval, etc.",
    },
    {
      id: "info",
      value: "/info",
      text: "Information",
      icon: "info",
      subheading: "App version and device information",
    },
    {
      id: "power",
      text: "Power",
      icon: "power_settings_new",
      subheading: "Reboot and enter photo display mode",
    },
  ];

  let confirmReboot = false;
  function selectListItem(e: { detail: string }) {
    const menuItem = menuItems.find(({ id }) => id == e.detail);
    if (menuItem?.value) {
      push(menuItem.value);
    } else if (menuItem?.id == "power") {
      confirmReboot = true;
    }
  }

  function reboot() {
    fetch("/api/v1/system/reboot", { method: "POST" }).then(() => {
      confirmReboot = false;
    });
  }
</script>

<main>
  <Container>
    <span slot="title">InkArt</span>
    <section>
      <List on:change={selectListItem} items={menuItems} class="w-full" />
    </section>
  </Container>
</main>

<Dialog value={confirmReboot}>
  <h5 slot="title">Reboot</h5>
  <div class="text-gray-700">Do you want to leave setup mode?</div>
  <div slot="actions">
    <Button text on:click={() => (confirmReboot = false)}>Cancel</Button>
    <Button text on:click={reboot} color="error">Reboot</Button>
  </div>
</Dialog>
